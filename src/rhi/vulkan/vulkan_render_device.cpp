// This define MUST be before including vulkan_render_device.hpp
#define VMA_IMPLEMENTATION
#include "vulkan_render_device.hpp"

#include <sstream>

#include <minitrace.h>
#include <rx/core/abort.h>
#include <rx/core/algorithm/max.h>
#include <rx/core/log.h>
#include <rx/core/set.h>
#include <signal.h>
#include <string.h>

#include "nova_renderer/camera.hpp"
#include "nova_renderer/constants.hpp"
#include "nova_renderer/frame_context.hpp"
#include "nova_renderer/renderables.hpp"
#include "nova_renderer/rhi/pipeline_create_info.hpp"
#include "nova_renderer/window.hpp"

#include "../../renderer/pipeline_reflection.hpp"
#include "vk_structs.hpp"
#include "vulkan_command_list.hpp"
#include "vulkan_resource_binder.hpp"
#include "vulkan_utils.hpp"

// TODO: Move window creation out of the RHI
#ifdef NOVA_LINUX
#define NOVA_VK_XLIB
#include "../../util/linux_utils.hpp"
#endif

using namespace nova::mem;
using rx::utility::move;

namespace nova::renderer::rhi {
    RX_LOG("VulkanRenderDevice", logger);

    constexpr auto REQUIRED_VULKAN_VERSION = VK_API_VERSION_1_1;

    void FencedTask::operator()() const { work_to_perform(); }

    VulkanRenderDevice::VulkanRenderDevice(NovaSettingsAccessManager& settings_in,
                                           NovaWindow& window_in,
                                           rx::memory::allocator& allocator) // NOLINT(cppcoreguidelines-pro-type-member-init)
        : RenderDevice{settings_in, window_in, allocator},
          command_pools_by_thread_idx{internal_allocator},
          fenced_tasks{internal_allocator},
          checkpoint_names{internal_allocator} {
        MTR_SCOPE("VulkanRenderDevice", "VulkanRenderDevice");

        create_instance();

        if(settings_in.settings.debug.enabled) {
            enable_debug_output();
        }

        create_surface();

        create_device_and_queues();

        save_device_info();

        initialize_vma();

        create_swapchain();

        create_per_thread_command_pools();

        create_standard_pipeline_layout();

        create_material_resource_binder();
    }

    void VulkanRenderDevice::set_num_renderpasses(uint32_t /* num_renderpasses */) {
        // Pretty sure Vulkan doesn't need to do anything here
    }

    rx::ptr<RhiRenderpass> VulkanRenderDevice::create_renderpass(const renderpack::RenderPassCreateInfo& data,
                                                                 const glm::uvec2& framebuffer_size,
                                                                 rx::memory::allocator& allocator) {
        MTR_SCOPE("VulkanRenderDevice", "create_renderpass");
        auto* vk_swapchain = static_cast<VulkanSwapchain*>(swapchain.get());
        VkExtent2D swapchain_extent = {swapchain_size.x, swapchain_size.y};

        auto* renderpass = allocator.create<VulkanRenderpass>();

        VkSubpassDescription subpass_description = {};
        subpass_description.flags = 0;
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.inputAttachmentCount = 0;
        subpass_description.pInputAttachments = nullptr;
        subpass_description.preserveAttachmentCount = 0;
        subpass_description.pPreserveAttachments = nullptr;
        subpass_description.pResolveAttachments = nullptr;
        subpass_description.pDepthStencilAttachment = nullptr;

        VkSubpassDependency image_available_dependency = {};
        image_available_dependency.dependencyFlags = 0;
        image_available_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        image_available_dependency.dstSubpass = 0;
        image_available_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        image_available_dependency.srcAccessMask = 0;
        image_available_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        image_available_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo render_pass_create_info = {};
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.pNext = nullptr;
        render_pass_create_info.flags = 0;
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass_description;
        render_pass_create_info.dependencyCount = 1;
        render_pass_create_info.pDependencies = &image_available_dependency;

        rx::vector<VkAttachmentReference> attachment_references{allocator};
        rx::vector<VkAttachmentDescription> attachments{allocator};
        rx::vector<VkImageView> framebuffer_attachments{allocator};
        uint32_t framebuffer_width = framebuffer_size.x;
        uint32_t framebuffer_height = framebuffer_size.y;

        bool writes_to_backbuffer = false;
        // Collect framebuffer size information from color output attachments
        data.texture_outputs.each_fwd([&](const renderpack::TextureAttachmentInfo& attachment) {
            if(attachment.name == BACKBUFFER_NAME) {
                // Handle backbuffer
                // Backbuffer framebuffers are handled by themselves in their own special snowflake way so we just need to skip
                // everything
                writes_to_backbuffer = true;

                VkAttachmentDescription desc;
                desc.flags = 0;
                desc.format = vk_swapchain->get_swapchain_format();
                desc.samples = VK_SAMPLE_COUNT_1_BIT;
                desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                attachments.push_back(desc);

                VkAttachmentReference ref;

                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                ref.attachment = static_cast<uint32_t>(attachments.size()) - 1;

                attachment_references.push_back(ref);

                framebuffer_width = swapchain_extent.width;
                framebuffer_height = swapchain_extent.height;

            } else {
                VkAttachmentDescription desc;
                desc.flags = 0;
                desc.format = to_vk_format(attachment.pixel_format);
                desc.samples = VK_SAMPLE_COUNT_1_BIT;
                desc.loadOp = attachment.clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
                desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                attachments.push_back(desc);

                VkAttachmentReference ref;
                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                ref.attachment = static_cast<uint32_t>(attachments.size()) - 1;

                attachment_references.push_back(ref);
            }
        });

        VkAttachmentReference depth_reference = {};
        // Collect framebuffer size information from the depth attachment
        if(data.depth_texture) {
            VkAttachmentDescription desc = {};
            desc.flags = 0;
            desc.format = to_vk_format(data.depth_texture->pixel_format);
            desc.samples = VK_SAMPLE_COUNT_1_BIT;
            desc.loadOp = data.depth_texture->clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            attachments.push_back(desc);

            depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depth_reference.attachment = static_cast<uint32_t>(attachments.size()) - 1;

            subpass_description.pDepthStencilAttachment = &depth_reference;
        }

        if(framebuffer_width == 0) {
            logger->error(
                "Framebuffer width for pass %s is 0. This is illegal! Make sure that there is at least one attachment for this render pass, and ensure that all attachments used by this pass have a non-zero width",
                data.name);
            return {};
        }

        if(framebuffer_height == 0) {
            logger->error(
                "Framebuffer height for pass %s is 0. This is illegal! Make sure that there is at least one attachment for this render pass, and ensure that all attachments used by this pass have a non-zero height",
                data.name);
            return {};
        }

        if(framebuffer_attachments.size() > gpu.props.limits.maxColorAttachments) {
            logger->error(
                "Framebuffer for pass %s has %d color attachments, but your GPU only supports %d. Please reduce the number of attachments that this pass uses, possibly by changing some of your input attachments to bound textures",
                data.name,
                data.texture_outputs.size(),
                gpu.props.limits.maxColorAttachments);
            return {};
        }

        subpass_description.colorAttachmentCount = static_cast<uint32_t>(attachment_references.size());
        subpass_description.pColorAttachments = attachment_references.data();

        render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_create_info.pAttachments = attachments.data();

        NOVA_CHECK_RESULT(vkCreateRenderPass(device, &render_pass_create_info, nullptr, &renderpass->pass));

        if(writes_to_backbuffer) {
            if(data.texture_outputs.size() > 1) {
                logger->error(
                    "Pass %s writes to the backbuffer, and other textures. Passes that write to the backbuffer are not allowed to write to any other textures",
                    data.name);
            }
        }

        renderpass->render_area = {{0, 0}, {framebuffer_width, framebuffer_height}};

        if(settings.settings.debug.enabled) {
            const auto object_name = vk::DebugUtilsObjectNameInfoEXT{}
                                         .setObjectType(vk::ObjectType::eRenderPass)
                                         .setObjectHandle(reinterpret_cast<uint64_t>(renderpass->pass))
                                         .setPObjectName(data.name.data());
            device.setDebugUtilsObjectNameEXT(&object_name, device_dynamic_loader);
        }

        return rx::ptr<RhiRenderpass>{allocator, renderpass};
    }

    rx::ptr<RhiFramebuffer> VulkanRenderDevice::create_framebuffer(const RhiRenderpass& renderpass,
                                                                   const rx::vector<RhiImage*>& color_attachments,
                                                                   RhiImage* depth_attachment,
                                                                   const glm::uvec2& framebuffer_size,
                                                                   rx::memory::allocator& allocator) {
        const auto& vk_renderpass = static_cast<const VulkanRenderpass&>(renderpass);

        rx::vector<VkImageView> attachment_views{allocator};
        attachment_views.reserve(color_attachments.size() + 1);

        color_attachments.each_fwd([&](const RhiImage* attachment) {
            const auto* vk_image = static_cast<const VulkanImage*>(attachment);
            attachment_views.push_back(vk_image->image_view);
        });

        // Depth attachment is ALWAYS the last attachment
        if(depth_attachment) {
            const auto* vk_depth_image = static_cast<const VulkanImage*>(depth_attachment);
            attachment_views.push_back(vk_depth_image->image_view);
        }

        VkFramebufferCreateInfo framebuffer_create_info = {};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = vk_renderpass.pass;
        framebuffer_create_info.attachmentCount = static_cast<uint32_t>(attachment_views.size());
        framebuffer_create_info.pAttachments = attachment_views.data();
        framebuffer_create_info.width = framebuffer_size.x;
        framebuffer_create_info.height = framebuffer_size.y;
        framebuffer_create_info.layers = 1;

        auto* framebuffer = allocator.create<VulkanFramebuffer>();
        framebuffer->size = framebuffer_size;
        framebuffer->num_attachments = static_cast<uint32_t>(attachment_views.size());

        NOVA_CHECK_RESULT(vkCreateFramebuffer(device, &framebuffer_create_info, nullptr, &framebuffer->framebuffer));

        return rx::ptr<RhiFramebuffer>{allocator, framebuffer};
    }

    rx::ptr<RhiPipeline> VulkanRenderDevice::create_surface_pipeline(const RhiGraphicsPipelineState& pipeline_state,
                                                                     rx::memory::allocator& allocator) {
        // For the Vulkan backend, creating a surface pipeline means creating a pipeline that uses the standard pipeline layout

        auto* pipeline = allocator.create<VulkanPipeline>();
        pipeline->name = pipeline_state.name;
        pipeline->layout.layout = standard_pipeline_layout;
        pipeline->layout.descriptor_set_layouts = {allocator, rx::array{standard_set_layout}};
        pipeline->layout.variable_descriptor_set_counts = {allocator, rx::array{MAX_NUM_TEXTURES}};
        pipeline->layout.bindings = standard_layout_bindings;
        pipeline->state = pipeline_state;

        return rx::ptr<RhiPipeline>{allocator, pipeline};
    }

    rx::ptr<RhiPipeline> VulkanRenderDevice::create_global_pipeline(const RhiGraphicsPipelineState& pipeline_state,
                                                                    rx::memory::allocator& allocator) {
        // For the Vulkan backend, creating a global pipeline means creating a pipeline with a bespoke pipeline layout

        const auto& layout = create_pipeline_layout(pipeline_state);

        auto* pipeline = allocator.create<VulkanPipeline>();
        pipeline->name = pipeline_state.name;
        pipeline->layout = layout;
        pipeline->state = pipeline_state;

        return rx::ptr<RhiPipeline>{allocator, pipeline};
    }

    rx::ptr<RhiResourceBinder> VulkanRenderDevice::create_resource_binder_for_pipeline(const RhiPipeline& pipeline,
                                                                                       rx::memory::allocator& allocator) {
        const auto& vk_pipeline = static_cast<const VulkanPipeline&>(pipeline);

        auto descriptors = create_descriptors(vk_pipeline.layout.descriptor_set_layouts,
                                              vk_pipeline.layout.variable_descriptor_set_counts,
                                              allocator);

        auto* binder = allocator.create<VulkanResourceBinder>(*this,
                                                              vk_pipeline.layout.bindings,
                                                              descriptors,
                                                              vk_pipeline.layout.layout,
                                                              allocator);

        return rx::ptr<RhiResourceBinder>{allocator, binder};
    }

    RhiResourceBinder* VulkanRenderDevice::get_material_resource_binder() { return material_resource_binder.get(); }

    rx::optional<vk::DescriptorPool> VulkanRenderDevice::create_descriptor_pool(
        const rx::map<DescriptorType, uint32_t>& descriptor_capacity, rx::memory::allocator& allocator) {
        MTR_SCOPE("VulkanRenderDevice", "create_descriptor_pool");
        rx::vector<vk::DescriptorPoolSize> pool_sizes{internal_allocator};

        uint32_t max_sets = 0;
        descriptor_capacity.each_pair([&](const DescriptorType& type, const uint32_t count) {
            pool_sizes.emplace_back(vk::DescriptorPoolSize{to_vk_descriptor_type(type), count});
            max_sets += count;
        });

        const auto pool_create_info = vk::DescriptorPoolCreateInfo()
                                          .setFlags(vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind)
                                          .setMaxSets(max_sets)
                                          .setPoolSizeCount(static_cast<uint32_t>(pool_sizes.size()))
                                          .setPPoolSizes(pool_sizes.data());

        vk::DescriptorPool pool;
        const auto result = device.createDescriptorPool(&pool_create_info, nullptr, &pool);
        if(result != vk::Result::eSuccess) {
            logger->error("Could not create descriptor pool: %s", vk::to_string(result).c_str());
        }

        return pool;
    }

    vk::DescriptorSet VulkanRenderDevice::get_next_standard_descriptor_set() {
        if(standard_descriptor_sets.is_empty()) {
            const auto variable_set_counts = rx::array{MAX_NUM_TEXTURES};
            const auto count_allocate_info = vk::DescriptorSetVariableDescriptorCountAllocateInfo()
                                                 .setPDescriptorCounts(variable_set_counts.data())
                                                 .setDescriptorSetCount(static_cast<uint32_t>(variable_set_counts.size()));

            const auto allocate_info = vk::DescriptorSetAllocateInfo()
                                           .setDescriptorPool(standard_descriptor_set_pool)
                                           .setDescriptorSetCount(1)
                                           .setPSetLayouts(&standard_set_layout)
                                           .setPNext(&count_allocate_info);

            vk::DescriptorSet set;
            device.allocateDescriptorSets(&allocate_info, &set);

            if(settings->debug.enabled) {
                const auto object_name = vk::DebugUtilsObjectNameInfoEXT{}
                                             .setObjectType(vk::ObjectType::eDescriptorSet)
                                             .setObjectHandle(reinterpret_cast<uint64_t>(static_cast<VkDescriptorSet>(set)))
                                             .setPObjectName("Standard descriptor set");

                device.setDebugUtilsObjectNameEXT(&object_name, device_dynamic_loader);
            }

            return set;

        } else {
            const auto set = standard_descriptor_sets.last();
            standard_descriptor_sets.pop_back();

            return set;
        }
    }

    void VulkanRenderDevice::return_standard_descriptor_sets(const rx::vector<vk::DescriptorSet>& sets) {
        standard_descriptor_sets += sets;
    }

    rx::vector<vk::DescriptorSet> VulkanRenderDevice::create_descriptors(const rx::vector<vk::DescriptorSetLayout>& descriptor_set_layouts,
                                                                         const rx::vector<uint32_t>& variable_descriptor_max_counts,
                                                                         rx::memory::allocator& allocator) const {
        RX_ASSERT(descriptor_set_layouts.size() == variable_descriptor_max_counts.size(),
                  "Descriptor set layous and varaible descriptor counts must be the same size");

        const auto variable_descriptor_counts = vk::DescriptorSetVariableDescriptorCountAllocateInfo()
                                                    .setDescriptorSetCount(static_cast<uint32_t>(variable_descriptor_max_counts.size()))
                                                    .setPDescriptorCounts(variable_descriptor_max_counts.data());

        const auto allocate_info = vk::DescriptorSetAllocateInfo()
                                       .setDescriptorSetCount(static_cast<uint32_t>(descriptor_set_layouts.size()))
                                       .setPSetLayouts(descriptor_set_layouts.data())
                                       .setDescriptorPool(standard_descriptor_set_pool)
                                       .setPNext(&variable_descriptor_counts);

        rx::vector<vk::DescriptorSet> sets{allocator, descriptor_set_layouts.size()};
        device.allocateDescriptorSets(&allocate_info, sets.data());

        return sets;
    }

    vk::Fence VulkanRenderDevice::get_next_submission_fence() {
        if(submission_fences.is_empty()) {
            const auto fence_create_info = vk::FenceCreateInfo();
            vk::Fence fence;
            device.createFence(&fence_create_info, nullptr, &fence);

            return fence;

        } else {
            const auto fence = submission_fences.last();
            submission_fences.pop_back();

            return fence;
        }
    }

    uint32_t VulkanRenderDevice::save_checkpoint_name(const rx::string& checkpoint_name) {
        const auto checkpoint_idx = checkpoint_names.size();
        checkpoint_names.push_back(checkpoint_name);

        return checkpoint_idx;
    }

    ntl::Result<vk::Pipeline> VulkanRenderDevice::compile_pipeline_state(const VulkanPipeline& pipeline_state,
                                                                         const VulkanRenderpass& renderpass,
                                                                         rx::memory::allocator& allocator) {
        MTR_SCOPE("VulkanRenderDevice", "create_pipeline");

        const auto& state = pipeline_state.state;

        logger->verbose("Creating a VkPipeline for pipeline %s", state.name);

        rx::vector<VkPipelineShaderStageCreateInfo> shader_stages{internal_allocator};
        rx::map<VkShaderStageFlags, VkShaderModule> shader_modules{internal_allocator};

        logger->verbose("Compiling vertex module");
        const auto vertex_module = create_shader_module(state.vertex_shader.source);
        if(vertex_module) {
            shader_modules.insert(VK_SHADER_STAGE_VERTEX_BIT, *vertex_module);
        } else {
            return ntl::Result<vk::Pipeline>{ntl::NovaError("Could not create vertex module")};
        }

        if(state.geometry_shader) {
            logger->verbose("Compiling geometry module");
            const auto geometry_module = create_shader_module(state.geometry_shader->source);
            if(geometry_module) {
                shader_modules.insert(VK_SHADER_STAGE_GEOMETRY_BIT, *geometry_module);
            } else {
                return ntl::Result<vk::Pipeline>{ntl::NovaError("Could not geometry module")};
            }
        }

        if(state.pixel_shader) {
            logger->verbose("Compiling fragment module");
            const auto fragment_module = create_shader_module(state.pixel_shader->source);
            if(fragment_module) {
                shader_modules.insert(VK_SHADER_STAGE_FRAGMENT_BIT, *fragment_module);
            } else {
                return ntl::Result<vk::Pipeline>{ntl::NovaError("Could not pixel module")};
            }

        } // namespace nova::renderer::rhi

        shader_modules.each_pair([&](const VkShaderStageFlags stage, const VkShaderModule shader_module) {
            VkPipelineShaderStageCreateInfo shader_stage_create_info;
            shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stage_create_info.pNext = nullptr;
            shader_stage_create_info.flags = 0;
            shader_stage_create_info.stage = static_cast<VkShaderStageFlagBits>(stage);
            shader_stage_create_info.module = shader_module;
            shader_stage_create_info.pName = "main";
            shader_stage_create_info.pSpecializationInfo = nullptr;

            shader_stages.push_back(shader_stage_create_info);
        });

        const auto& [vertex_attribute_descriptions, vertex_binding_descriptions] = get_input_assembler_setup(state.vertex_fields);

        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info;
        vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.pNext = nullptr;
        vertex_input_state_create_info.flags = 0;
        vertex_input_state_create_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_binding_descriptions.size());
        vertex_input_state_create_info.pVertexBindingDescriptions = vertex_binding_descriptions.data();
        vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_descriptions.size());
        vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info;
        input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.pNext = nullptr;
        input_assembly_create_info.flags = 0;
        input_assembly_create_info.primitiveRestartEnable = VK_FALSE;
        switch(state.topology) {
            case PrimitiveTopology::TriangleList:
                input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                break;

            case PrimitiveTopology::LineList:
                input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                break;

            case PrimitiveTopology::PointList:
                input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
                break;
        }

        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<float>(renderpass.render_area.extent.width);
        viewport.height = static_cast<float>(renderpass.render_area.extent.height);
        viewport.minDepth = 0.0F;
        viewport.maxDepth = 1.0F;

        VkRect2D scissor;
        scissor.offset = {0, 0};
        scissor.extent = {static_cast<uint32_t>(state.viewport_size.x), static_cast<uint32_t>(state.viewport_size.y)};

        VkPipelineViewportStateCreateInfo viewport_state_create_info;
        viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.pNext = nullptr;
        viewport_state_create_info.flags = 0;
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.pViewports = &viewport;
        viewport_state_create_info.scissorCount = 1;
        viewport_state_create_info.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer_create_info;
        rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer_create_info.pNext = nullptr;
        rasterizer_create_info.flags = 0;
        rasterizer_create_info.depthClampEnable = VK_FALSE;
        rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
        rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer_create_info.lineWidth = 1.0F;
        rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer_create_info.depthClampEnable = VK_FALSE;
        rasterizer_create_info.depthBiasConstantFactor = state.rasterizer_state.depth_bias;
        rasterizer_create_info.depthBiasSlopeFactor = state.rasterizer_state.slope_scaled_depth_bias;
        rasterizer_create_info.depthBiasClamp = state.rasterizer_state.maximum_depth_bias;

        if(rasterizer_create_info.depthBiasConstantFactor != 0 || rasterizer_create_info.depthBiasSlopeFactor != 0) {
            rasterizer_create_info.depthBiasEnable = VK_TRUE;

        } else {
            rasterizer_create_info.depthBiasEnable = VK_FALSE;
        }

        VkPipelineMultisampleStateCreateInfo multisample_create_info;
        multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_create_info.pNext = nullptr;
        multisample_create_info.flags = 0;
        multisample_create_info.sampleShadingEnable = VK_FALSE;
        multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisample_create_info.minSampleShading = 1.0F;
        multisample_create_info.pSampleMask = nullptr;
        multisample_create_info.alphaToCoverageEnable = VK_FALSE;
        multisample_create_info.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
        depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

        if(state.depth_state) {
            const auto& depth_state = *state.depth_state;

            depth_stencil_create_info.depthTestEnable = VK_TRUE;
            depth_stencil_create_info.depthWriteEnable = static_cast<VkBool32>(depth_state.enable_depth_write);
            depth_stencil_create_info.depthCompareOp = to_compare_op(depth_state.compare_op);

            if(depth_state.bounds_test_state) {
                depth_stencil_create_info.depthBoundsTestEnable = VK_TRUE;
                if(depth_state.bounds_test_state->mode == DepthBoundsTestMode::Static) {
                    depth_stencil_create_info.minDepthBounds = depth_state.bounds_test_state->static_state.min_bound;
                    depth_stencil_create_info.maxDepthBounds = depth_state.bounds_test_state->static_state.max_bound;
                }
            }
        }

        if(state.stencil_state) {
            const auto stencil_state = *state.stencil_state;

            depth_stencil_create_info.stencilTestEnable = VK_TRUE;

            depth_stencil_create_info.front.failOp = to_stencil_op(stencil_state.front_face_op.fail_op);
            depth_stencil_create_info.front.passOp = to_stencil_op(stencil_state.front_face_op.pass_op);
            depth_stencil_create_info.front.depthFailOp = to_stencil_op(stencil_state.front_face_op.depth_fail_op);
            depth_stencil_create_info.front.compareOp = to_compare_op(stencil_state.front_face_op.compare_op);
            depth_stencil_create_info.front.compareMask = stencil_state.compare_mask;
            depth_stencil_create_info.front.writeMask = stencil_state.write_mask;

            depth_stencil_create_info.back.failOp = to_stencil_op(stencil_state.back_face_op.fail_op);
            depth_stencil_create_info.back.passOp = to_stencil_op(stencil_state.back_face_op.pass_op);
            depth_stencil_create_info.back.depthFailOp = to_stencil_op(stencil_state.back_face_op.depth_fail_op);
            depth_stencil_create_info.back.compareOp = to_compare_op(stencil_state.back_face_op.compare_op);
            depth_stencil_create_info.back.compareMask = stencil_state.compare_mask;
            depth_stencil_create_info.back.writeMask = stencil_state.write_mask;
        }

        VkPipelineColorBlendStateCreateInfo color_blend_create_info{};
        color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_create_info.pNext = nullptr;
        color_blend_create_info.flags = 0;
        color_blend_create_info.logicOpEnable = VK_FALSE;
        color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;

        rx::vector<VkPipelineColorBlendAttachmentState> attachment_states{allocator};
        if(state.blend_state) {
            const auto& blend_state = *state.blend_state;

            attachment_states.reserve(blend_state.render_target_states.size());

            blend_state.render_target_states.each_fwd([&](const RenderTargetBlendState& render_target_blend) {
                VkPipelineColorBlendAttachmentState color_blend_attachment;
                color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                                        VK_COLOR_COMPONENT_A_BIT;
                color_blend_attachment.blendEnable = render_target_blend.enable ? VK_TRUE : VK_FALSE;
                color_blend_attachment.srcColorBlendFactor = to_blend_factor(render_target_blend.src_color_factor);
                color_blend_attachment.dstColorBlendFactor = to_blend_factor(render_target_blend.dst_color_factor);
                color_blend_attachment.colorBlendOp = to_blend_op(render_target_blend.color_op);
                color_blend_attachment.srcAlphaBlendFactor = to_blend_factor(render_target_blend.src_alpha_factor);
                color_blend_attachment.dstAlphaBlendFactor = to_blend_factor(render_target_blend.dst_alpha_factor);
                color_blend_attachment.alphaBlendOp = to_blend_op(render_target_blend.alpha_op);

                attachment_states.emplace_back(color_blend_attachment);
            });

            color_blend_create_info.attachmentCount = static_cast<uint32_t>(attachment_states.size());
            color_blend_create_info.pAttachments = attachment_states.data();
            color_blend_create_info.blendConstants[0] = blend_state.blend_constants.r;
            color_blend_create_info.blendConstants[1] = blend_state.blend_constants.g;
            color_blend_create_info.blendConstants[2] = blend_state.blend_constants.b;
            color_blend_create_info.blendConstants[3] = blend_state.blend_constants.a;

        } else {
            attachment_states.reserve(state.color_attachments.size());

            state.color_attachments.each_fwd([&](const renderpack::TextureAttachmentInfo& /* attachment_info */) {
                VkPipelineColorBlendAttachmentState color_blend_attachment{};
                color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                                        VK_COLOR_COMPONENT_A_BIT;
                color_blend_attachment.blendEnable = VK_FALSE;

                attachment_states.emplace_back(color_blend_attachment);
            });

            color_blend_create_info.attachmentCount = static_cast<uint32_t>(attachment_states.size());
            color_blend_create_info.pAttachments = attachment_states.data();
        }

        rx::vector<VkDynamicState> dynamic_states;

        if(state.enable_scissor_test) {
            dynamic_states.emplace_back(VK_DYNAMIC_STATE_SCISSOR);
        }

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
        dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
        dynamic_state_create_info.pDynamicStates = dynamic_states.data();

        VkGraphicsPipelineCreateInfo pipeline_create_info = {};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_create_info.pNext = nullptr;
        pipeline_create_info.flags = 0;
        pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
        pipeline_create_info.pStages = shader_stages.data();
        pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
        pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
        pipeline_create_info.pViewportState = &viewport_state_create_info;
        pipeline_create_info.pRasterizationState = &rasterizer_create_info;
        pipeline_create_info.pMultisampleState = &multisample_create_info;
        pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
        pipeline_create_info.pColorBlendState = &color_blend_create_info;
        pipeline_create_info.pDynamicState = &dynamic_state_create_info;
        pipeline_create_info.layout = pipeline_state.layout.layout;

        pipeline_create_info.renderPass = renderpass.pass;
        pipeline_create_info.subpass = 0;
        pipeline_create_info.basePipelineIndex = -1;

        vk::Pipeline pipeline;
        const auto result = vkCreateGraphicsPipelines(device,
                                                      nullptr,
                                                      1,
                                                      &pipeline_create_info,
                                                      nullptr,
                                                      reinterpret_cast<VkPipeline*>(&pipeline));
        if(result != VK_SUCCESS) {
            return ntl::Result<vk::Pipeline>{MAKE_ERROR("Could not compile pipeline %s", state.name)};
        }

        if(settings.settings.debug.enabled) {
            const auto object_name = vk::DebugUtilsObjectNameInfoEXT{}
                                         .setObjectType(vk::ObjectType::ePipeline)
                                         .setObjectHandle(reinterpret_cast<uint64_t>(static_cast<VkPipeline>(pipeline)))
                                         .setPObjectName(state.name.data());

            device.setDebugUtilsObjectNameEXT(&object_name, device_dynamic_loader);
        }

        return ntl::Result{pipeline};
    }

    rx::ptr<RhiBuffer> VulkanRenderDevice::create_buffer(const RhiBufferCreateInfo& info, rx::memory::allocator& allocator) {
        MTR_SCOPE("VulkanRenderDevice", "create_buffer");
        auto* buffer = allocator.create<VulkanBuffer>();

        VkBufferCreateInfo vk_create_info = {};
        vk_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vk_create_info.size = info.size.b_count();
        vk_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo vma_alloc{};

        switch(info.buffer_usage) {
            case BufferUsage::UniformBuffer: {
                if(info.size < gpu.props.limits.maxUniformBufferRange) {
                    vk_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

                } else {
                    vk_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
                }
                vma_alloc.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
                vma_alloc.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            } break;

            case BufferUsage::IndexBuffer: {
                vk_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
                vma_alloc.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            } break;

            case BufferUsage::VertexBuffer: {
                vk_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                vma_alloc.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            } break;

            case BufferUsage::StagingBuffer: {
                vk_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                vma_alloc.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
                vma_alloc.usage = VMA_MEMORY_USAGE_CPU_ONLY;
            } break;
        }

        const auto result = vmaCreateBuffer(vma,
                                            &vk_create_info,
                                            &vma_alloc,
                                            &buffer->buffer,
                                            &buffer->allocation,
                                            &buffer->allocation_info);
        if(result == VK_SUCCESS) {
            buffer->size = info.size;

            if(settings->debug.enabled) {
                const auto object_name = vk::DebugUtilsObjectNameInfoEXT()
                                             .setObjectType(vk::ObjectType::eBuffer)
                                             .setObjectHandle(reinterpret_cast<uint64_t>(buffer->buffer))
                                             .setPObjectName(info.name.data());
                device.setDebugUtilsObjectNameEXT(&object_name, device_dynamic_loader);
            }

            return rx::ptr<RhiBuffer>{allocator, buffer};

        } else {
            logger->error("Could not create buffer %s: %s", info.name, to_string(result));

            return {};
        }
    }

    void VulkanRenderDevice::write_data_to_buffer(const void* data, const Bytes num_bytes, const RhiBuffer& buffer) {
        MTR_SCOPE("VulkanRenderDevice", "write_data_to_buffer");
        const auto& vulkan_buffer = static_cast<const VulkanBuffer&>(buffer);

        memcpy(vulkan_buffer.allocation_info.pMappedData, data, num_bytes.b_count());
    }

    rx::ptr<RhiSampler> VulkanRenderDevice::create_sampler(const RhiSamplerCreateInfo& create_info, rx::memory::allocator& allocator) {
        MTR_SCOPE("VulkanRenderDevice", "create_sampler");
        auto* sampler = allocator.create<VulkanSampler>();

        VkSamplerCreateInfo vk_create_info = {};
        vk_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        vk_create_info.minFilter = to_vk_filter(create_info.min_filter);
        vk_create_info.magFilter = to_vk_filter(create_info.mag_filter);
        vk_create_info.addressModeU = to_vk_address_mode(create_info.x_wrap_mode);
        vk_create_info.addressModeV = to_vk_address_mode(create_info.y_wrap_mode);
        vk_create_info.addressModeW = to_vk_address_mode(create_info.z_wrap_mode);
        vk_create_info.mipLodBias = create_info.mip_bias;
        vk_create_info.anisotropyEnable = create_info.enable_anisotropy ? VK_TRUE : VK_FALSE;
        vk_create_info.maxAnisotropy = create_info.max_anisotropy;
        vk_create_info.minLod = create_info.min_lod;
        vk_create_info.maxLod = create_info.max_lod;

        vkCreateSampler(device, &vk_create_info, nullptr, &sampler->sampler);

        return rx::ptr<RhiSampler>(allocator, sampler);
    }

    rx::ptr<RhiImage> VulkanRenderDevice::create_image(const renderpack::TextureCreateInfo& info, rx::memory::allocator& allocator) {
        MTR_SCOPE("VulkanRenderDevice", "create_image");
        auto* image = allocator.create<VulkanImage>();

        image->is_dynamic = true;
        image->type = ResourceType::Image;
        const VkFormat format = to_vk_format(info.format.pixel_format);

        // In Nova, images all have a dedicated allocation
        // This may or may not change depending on performance data, but given Nova's atlas-centric design I don't think it'll change much
        const auto image_pixel_size = info.format.get_size_in_pixels(swapchain_size);

        VmaAllocationCreateInfo vma_info = {};
        vma_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkImageCreateInfo image_create_info = {};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = format;
        image_create_info.extent.width = image_pixel_size.x;
        image_create_info.extent.height = image_pixel_size.y;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

        if(format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT) {
            image->is_depth_tex = true;
        }

        if(info.usage == renderpack::ImageUsage::SampledImage) {
            image_create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        } else {
            // If the image isn't a sampled image, it's a render target
            // Render targets get dedicated allocations

            if(format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT) {
                image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            } else {
                image_create_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }

            vma_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        }

        image_create_info.queueFamilyIndexCount = 1;
        image_create_info.pQueueFamilyIndices = &graphics_family_index;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        const auto result = vmaCreateImage(vma, &image_create_info, &vma_info, &image->image, &image->allocation, nullptr);
        if(result == VK_SUCCESS) {
            if(settings->debug.enabled) {
                const auto object_name = vk::DebugUtilsObjectNameInfoEXT{}
                                             .setObjectType(vk::ObjectType::eImage)
                                             .setObjectHandle(reinterpret_cast<uint64_t>(image->image))
                                             .setPObjectName(info.name.data());

                device.setDebugUtilsObjectNameEXT(&object_name, device_dynamic_loader);
            }

            VkImageViewCreateInfo image_view_create_info = {};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.image = image->image;
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = image_create_info.format;
            if(format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT) {
                image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            } else {
                image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;

            vkCreateImageView(device, &image_view_create_info, nullptr, &image->image_view);

            return rx::ptr<RhiImage>{allocator, image};

        } else {
            logger->error("Could not create image %s: %s", info.name, to_string(result));

            return {};
        }
    }

    rx::ptr<RhiSemaphore> VulkanRenderDevice::create_semaphore(rx::memory::allocator& allocator) {
        MTR_SCOPE("VulkanRenderDevice", "create_semaphore");
        auto* semaphore = allocator.create<VulkanSemaphore>();

        VkSemaphoreCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        vkCreateSemaphore(device, &create_info, nullptr, &semaphore->semaphore);

        return rx::ptr<RhiSemaphore>{allocator, semaphore};
    }

    rx::vector<rx::ptr<RhiSemaphore>> VulkanRenderDevice::create_semaphores(const uint32_t num_semaphores,
                                                                            rx::memory::allocator& allocator) {
        MTR_SCOPE("VulkanRenderDevice", "create_semaphores");
        auto semaphores = rx::vector<rx::ptr<RhiSemaphore>>{allocator};
        semaphores.reserve(num_semaphores);

        for(uint32_t i = 0; i < num_semaphores; i++) {
            semaphores.push_back(move(create_semaphore(allocator)));
        }

        return semaphores;
    }

    rx::ptr<RhiFence> VulkanRenderDevice::create_fence(const bool signaled, rx::memory::allocator& allocator) {
        MTR_SCOPE("VulkanRenderDevice", "create_fence");
        auto fence = rx::make_ptr<VulkanFence>(allocator);

        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        if(signaled) {
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }

        vkCreateFence(device, &fence_create_info, nullptr, &fence->fence);

        return fence;
    }

    rx::vector<rx::ptr<RhiFence>> VulkanRenderDevice::create_fences(const uint32_t num_fences,
                                                                    const bool signaled,
                                                                    rx::memory::allocator& allocator) {
        MTR_SCOPE("VulkanRenderDevice", "create_fences");
        rx::vector<rx::ptr<RhiFence>> fences{allocator};
        fences.reserve(num_fences);

        for(uint32_t i = 0; i < num_fences; i++) {
            auto fence = create_fence(signaled, allocator);

            fences.push_back(move(fence));
        }

        return fences;
    }

    void VulkanRenderDevice::wait_for_fences(const rx::vector<RhiFence*>& fences) {
        MTR_SCOPE("VulkanRenderDevice", "wait_for_fences");
        rx::vector<VkFence> vk_fences{internal_allocator};
        vk_fences.reserve(fences.size());
        fences.each_fwd([&](const RhiFence* fence) {
            const auto* vk_fence = static_cast<const VulkanFence*>(fence);
            vk_fences.push_back(vk_fence->fence);
        });

        const auto result = vkWaitForFences(device,
                                            static_cast<uint32_t>(vk_fences.size()),
                                            vk_fences.data(),
                                            VK_TRUE,
                                            std::numeric_limits<uint64_t>::max());

        if(settings->debug.enabled) {
            if(result != VK_SUCCESS) {
                logger->error("Could not wait for fences. %s (error code %x)", to_string(result), result);
            }
        }
    }

    void VulkanRenderDevice::reset_fences(const rx::vector<RhiFence*>& fences) {
        MTR_SCOPE("VulkanRenderDevice", "reset_fences");
        rx::vector<VkFence> vk_fences{internal_allocator};
        vk_fences.reserve(fences.size());
        fences.each_fwd([&](const RhiFence* fence) {
            const auto* vk_fence = static_cast<const VulkanFence*>(fence);
            vk_fences.push_back(vk_fence->fence);
        });

        vkResetFences(device, static_cast<uint32_t>(fences.size()), vk_fences.data());
    }

    void VulkanRenderDevice::destroy_renderpass(rx::ptr<RhiRenderpass> pass) {
        MTR_SCOPE("VulkanRenderDevice", "destroy_renderpasses");
        auto* vk_renderpass = static_cast<VulkanRenderpass*>(pass.get());
        vkDestroyRenderPass(device, vk_renderpass->pass, nullptr);
    }

    void VulkanRenderDevice::destroy_framebuffer(rx::ptr<RhiFramebuffer> framebuffer) {
        MTR_SCOPE("VulkanRenderDevice", "destroy_framebuffer");
        const auto* vk_framebuffer = static_cast<const VulkanFramebuffer*>(framebuffer.get());
        vkDestroyFramebuffer(device, vk_framebuffer->framebuffer, nullptr);
    }

    void VulkanRenderDevice::destroy_texture(rx::ptr<RhiImage> resource) {
        MTR_SCOPE("VulkanRenderDevice", "destroy_texture");
        auto* vk_image = static_cast<VulkanImage*>(resource.get());
        vmaDestroyImage(vma, vk_image->image, vk_image->allocation);
    }

    void VulkanRenderDevice::destroy_semaphores(rx::vector<rx::ptr<RhiSemaphore>>& semaphores) {
        MTR_SCOPE("VulkanRenderDevice", "destroy_semaphores");
        semaphores.each_fwd([&](rx::ptr<RhiSemaphore>& semaphore) {
            auto* vk_semaphore = static_cast<VulkanSemaphore*>(semaphore.get());
            vkDestroySemaphore(device, vk_semaphore->semaphore, nullptr);
        });
    }

    void VulkanRenderDevice::destroy_fences(const rx::vector<rx::ptr<RhiFence>>& fences) {
        MTR_SCOPE("VulkanRenderDevice", "destroy_fences");
        fences.each_fwd([&](rx::ptr<RhiFence>& fence) {
            auto* vk_fence = static_cast<VulkanFence*>(fence.get());
            vkDestroyFence(device, vk_fence->fence, nullptr);
        });
    }

    rx::ptr<RhiRenderCommandList> VulkanRenderDevice::create_command_list(const uint32_t thread_idx,
                                                                          const QueueType needed_queue_type,
                                                                          const RhiRenderCommandList::Level level,
                                                                          rx::memory::allocator& allocator) {
        MTR_SCOPE("VulkanRenderDevice", "create_command_list");
        const uint32_t queue_family_index = get_queue_family_index(needed_queue_type);
        const VkCommandPool pool = *command_pools_by_thread_idx[thread_idx].find(queue_family_index);

        VkCommandBufferAllocateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        create_info.commandPool = pool;
        create_info.level = to_vk_command_buffer_level(level);
        create_info.commandBufferCount = 1;

        VkCommandBuffer new_buffer;
        vkAllocateCommandBuffers(device, &create_info, &new_buffer);

        auto* list = allocator.create<VulkanRenderCommandList>(new_buffer, *this, allocator);

        return rx::ptr<RhiRenderCommandList>{allocator, list};
    }

    void VulkanRenderDevice::submit_command_list(const rx::ptr<RhiRenderCommandList> cmds,
                                                 const QueueType queue,
                                                 RhiFence* fence_to_signal,
                                                 const rx::vector<RhiSemaphore*>& wait_semaphores,
                                                 const rx::vector<RhiSemaphore*>& signal_semaphores) {
        MTR_SCOPE("VulkanRenderDevice", "submit_command_list");
        auto* vk_list = static_cast<VulkanRenderCommandList*>(cmds.get());
        vkEndCommandBuffer(vk_list->cmds);

        VkQueue queue_to_submit_to;

        switch(queue) {
            case QueueType::Graphics:
                queue_to_submit_to = graphics_queue;
                break;

            case QueueType::Transfer:
                queue_to_submit_to = copy_queue;
                break;

            case QueueType::AsyncCompute:
                queue_to_submit_to = compute_queue;
                break;

            default:
                queue_to_submit_to = graphics_queue;
        }

        rx::vector<VkSemaphore> vk_wait_semaphores{internal_allocator};
        vk_wait_semaphores.reserve(wait_semaphores.size());
        wait_semaphores.each_fwd([&](const RhiSemaphore* semaphore) {
            const auto* vk_semaphore = static_cast<const VulkanSemaphore*>(semaphore);
            vk_wait_semaphores.push_back(vk_semaphore->semaphore);
        });

        rx::vector<VkSemaphore> vk_signal_semaphores{internal_allocator};
        vk_signal_semaphores.reserve(signal_semaphores.size());
        signal_semaphores.each_fwd([&](const RhiSemaphore* semaphore) {
            const auto* vk_semaphore = static_cast<const VulkanSemaphore*>(semaphore);
            vk_signal_semaphores.push_back(vk_semaphore->semaphore);
        });

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = static_cast<uint32_t>(vk_wait_semaphores.size());
        submit_info.pWaitSemaphores = vk_wait_semaphores.data();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &vk_list->cmds;
        submit_info.signalSemaphoreCount = static_cast<uint32_t>(vk_signal_semaphores.size());
        submit_info.pSignalSemaphores = vk_signal_semaphores.data();

        vk::Fence vk_signal_fence;
        if(fence_to_signal) {
            const auto* vk_fence = static_cast<const VulkanFence*>(fence_to_signal);
            vk_signal_fence = vk_fence->fence;

        } else {
            vk_signal_fence = get_next_submission_fence();
        }

        const auto result = vkQueueSubmit(queue_to_submit_to, 1, &submit_info, static_cast<VkFence>(vk_signal_fence));

        if(settings->debug.enabled) {
            if(result != VK_SUCCESS) {
                logger->error("Could not submit command list: %s", to_string(result));

                if(has_nv_device_checkpoints) {
                    uint32_t num_checkpoints;
                    device_dynamic_loader.vkGetQueueCheckpointDataNV(queue_to_submit_to, &num_checkpoints, nullptr);
                    rx::vector<VkCheckpointDataNV> checkpoint_data{internal_allocator, num_checkpoints};
                    checkpoint_data.each_fwd([](VkCheckpointDataNV& data) { data.sType = VK_STRUCTURE_TYPE_CHECKPOINT_DATA_NV; });
                    device_dynamic_loader.vkGetQueueCheckpointDataNV(queue_to_submit_to, &num_checkpoints, checkpoint_data.data());

                    checkpoint_data.each_fwd([&](const VkCheckpointDataNV& data) {
                        const auto stages = vk::to_string(vk::PipelineStageFlagBits{static_cast<VkPipelineStageFlags>(data.stage)});
                        const auto checkpoint_name = checkpoint_names[reinterpret_cast<rx_size>(data.pCheckpointMarker)];
                        logger->error("Pipeline stage %s reached checkpoint %s", stages.c_str(), checkpoint_name);
                    });
                }

                rx::abort("VK_DEVICE_LOST on queue submit");
            }
        }

        fenced_tasks.emplace_back(vk_signal_fence, [&] {
            vk_list->cleanup_resources();
            submission_fences.emplace_back(vk_signal_fence);
        });
    }

    void VulkanRenderDevice::end_frame(FrameContext& /* ctx */) {
        MTR_SCOPE("VulkanRenderEngine", "end_frame");
        // Intentionally copying the vector
        auto cur_tasks = fenced_tasks;

        // Clear out the list of tasks. We've copied the tasks to the new vector so it's fine, and we'll add back in the tasks that aren't
        // ready to run
        fenced_tasks.clear();

        cur_tasks.each_fwd([&](const FencedTask& task) {
            if(device.getFenceStatus(task.fence) == vk::Result::eSuccess) {
                task();

            } else {
                fenced_tasks.push_back(task);
            }
        });
    }

    uint32_t VulkanRenderDevice::get_queue_family_index(const QueueType type) const {
        switch(type) {
            case QueueType::Graphics:
                return graphics_family_index;

            case QueueType::Transfer:
                return transfer_family_index;

            case QueueType::AsyncCompute:
                return compute_family_index;

            default:
                RX_ASSERT(false, "Unknown queue type %u", static_cast<uint32_t>(type));
        }
    }

    VulkanPipelineLayoutInfo VulkanRenderDevice::create_pipeline_layout(const RhiGraphicsPipelineState& state) {
        const auto bindings = get_all_descriptors(state);
        const auto ds_layouts = create_descriptor_set_layouts(bindings, *this, internal_allocator);

        const auto pipeline_layout_create = vk::PipelineLayoutCreateInfo()
                                                .setSetLayoutCount(static_cast<uint32_t>(ds_layouts.size()))
                                                .setPSetLayouts(ds_layouts.data())
                                                .setPushConstantRangeCount(static_cast<uint32_t>(standard_push_constants.size()))
                                                .setPPushConstantRanges(standard_push_constants.data()); // TODO: Get this from reflection

        vk::PipelineLayout layout;
        device.createPipelineLayout(&pipeline_layout_create, nullptr, &layout);

        rx::vector<uint32_t> variable_descriptor_counts{internal_allocator, ds_layouts.size()};
        bindings.each_value([&](const RhiResourceBindingDescription& binding_desc) {
            if(binding_desc.is_unbounded && binding_desc.count > variable_descriptor_counts[binding_desc.set]) {
                variable_descriptor_counts[binding_desc.set] = binding_desc.count;
            }
        });

        return {bindings, ds_layouts, layout, variable_descriptor_counts};
    }

    void VulkanRenderDevice::create_surface() {
        MTR_SCOPE("VulkanRenderDevice", "create_surface");
#ifdef NOVA_LINUX
        VkXlibSurfaceCreateInfoKHR x_surface_create_info;
        x_surface_create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        x_surface_create_info.pNext = nullptr;
        x_surface_create_info.flags = 0;

        x_surface_create_info.dpy = window.get_display();
        x_surface_create_info.window = window.get_window_handle();

        NOVA_CHECK_RESULT(vkCreateXlibSurfaceKHR(instance, &x_surface_create_info, nullptr, &surface));

#elif defined(NOVA_WINDOWS)
        VkWin32SurfaceCreateInfoKHR win32_surface_create = {};
        win32_surface_create.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32_surface_create.hwnd = window.get_window_handle();

        NOVA_CHECK_RESULT(vkCreateWin32SurfaceKHR(instance, &win32_surface_create, nullptr, &surface));

#else
#error Unsuported window system
#endif
    }

    void VulkanRenderDevice::create_instance() {
        MTR_SCOPE("VulkanRenderDevice", "create_instance");

        const auto& version = settings.settings.vulkan.application_version;

        VkApplicationInfo application_info;
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pNext = nullptr;
        application_info.pApplicationName = settings.settings.vulkan.application_name;
        application_info.applicationVersion = VK_MAKE_VERSION(version.major, version.minor, version.patch);
        application_info.pEngineName = "Nova Renderer 0.9";
        application_info.apiVersion = REQUIRED_VULKAN_VERSION;

        VkInstanceCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.pApplicationInfo = &application_info;

        if(settings.settings.debug.enabled && settings.settings.debug.enable_validation_layers) {
            enabled_layer_names.push_back("VK_LAYER_LUNARG_standard_validation");
        }
        create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layer_names.size());
        create_info.ppEnabledLayerNames = enabled_layer_names.data();

        rx::vector<const char*> enabled_extension_names{internal_allocator};
        enabled_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        enabled_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef NOVA_LINUX
        enabled_extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(NOVA_WINDOWS)
        enabled_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
#error Unsupported Operating system
#endif

        rx::vector<VkValidationFeatureEnableEXT> enabled_validation_features;

        if(settings.settings.debug.enabled) {
            enabled_extension_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            enabled_extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

            enabled_validation_features.push_back(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT);

            if(settings.settings.debug.enable_gpu_based_validation) {
                enabled_validation_features.push_back(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT);
                enabled_validation_features.push_back(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT);
            }
        }

        create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extension_names.size());
        create_info.ppEnabledExtensionNames = enabled_extension_names.data();

        VkValidationFeaturesEXT validation_features = {};
        validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        validation_features.enabledValidationFeatureCount = static_cast<uint32_t>(enabled_validation_features.size());
        validation_features.pEnabledValidationFeatures = enabled_validation_features.data();

        create_info.pNext = &validation_features;

        {
            MTR_SCOPE("VulkanRenderDevice", "vkCreateInstance");
            NOVA_CHECK_RESULT(vkCreateInstance(&create_info, nullptr, &instance));
        }

        instance_dynamic_loader = {instance, vkGetInstanceProcAddr};
    }

    void VulkanRenderDevice::enable_debug_output() {
        MTR_SCOPE("VulkanRenderDevice", "enable_debug_output");

        const auto debug_create_info = vk::DebugUtilsMessengerCreateInfoEXT{}
                                           .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
                                           .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
                                           .setPfnUserCallback(
                                               reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(&debug_report_callback))
                                           .setPUserData(this);

        vk::Instance{instance}.createDebugUtilsMessengerEXT(&debug_create_info, nullptr, &debug_callback, instance_dynamic_loader);
    }

    void VulkanRenderDevice::save_device_info() {
        MTR_SCOPE("VulkanRenderDevice", "save_device_info");
        switch(gpu.props.vendorID) {
            case AMD_PCI_VENDOR_ID:
                device_info.architecture = DeviceArchitecture::Amd;
                break;

            case INTEL_PCI_VENDOR_ID:
                device_info.architecture = DeviceArchitecture::Intel;
                break;

            case NVIDIA_PCI_VENDOR_ID:
                device_info.architecture = DeviceArchitecture::Nvidia;
                break;

            default:
                device_info.architecture = DeviceArchitecture::Unknown;
        }

        vk_info.max_uniform_buffer_size = gpu.props.limits.maxUniformBufferRange;
        device_info.max_texture_size = gpu.props.limits.maxImageDimension2D;

        // TODO: Something smarter when Intel releases discreet GPUS
        // TODO: Handle integrated AMD GPUs
        device_info.is_uma = device_info.architecture == DeviceArchitecture::Intel;

        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(gpu.phys_device, nullptr, &extension_count, nullptr);
        rx::vector<VkExtensionProperties> available_extensions{internal_allocator, extension_count};
        vkEnumerateDeviceExtensionProperties(gpu.phys_device, nullptr, &extension_count, available_extensions.data());

        const auto extension_name_matcher = [](const char* ext_name) {
            return [=](const VkExtensionProperties& ext_props) -> bool { return strcmp(ext_name, ext_props.extensionName) == 0; };
        };

        // TODO: Update as more GPUs support hardware raytracing
        device_info.supports_raytracing = available_extensions.find_if(extension_name_matcher(VK_NV_RAY_TRACING_EXTENSION_NAME)) !=
                                          rx::vector<VkExtensionProperties>::k_npos;

        // TODO: Update as more GPUs support mesh shaders
        device_info.supports_mesh_shaders = available_extensions.find_if(extension_name_matcher(VK_NV_MESH_SHADER_EXTENSION_NAME));
    }

    void VulkanRenderDevice::initialize_vma() {
        MTR_SCOPE("VulkanRenderDevice", "initialize_vma");

        VmaAllocatorCreateInfo create_info{};
        create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        create_info.physicalDevice = gpu.phys_device;
        create_info.device = device;
        create_info.instance = instance;

        const auto result = vmaCreateAllocator(&create_info, &vma);

        if(result != VK_SUCCESS) {
            logger->error("Could not initialize VMA: %s", to_string(result));
        }
    }

    void VulkanRenderDevice::create_device_and_queues() {
        MTR_SCOPE("VulkanRenderDevice", "create_device_and_queues");
        rx::vector<const char*> device_extensions{internal_allocator};
        device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        device_extensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
        device_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

        uint32_t device_count;
        NOVA_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));
        auto physical_devices = rx::vector<VkPhysicalDevice>{internal_allocator, device_count};
        NOVA_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));

        uint32_t graphics_family_idx = 0xFFFFFFFF;
        uint32_t compute_family_idx = 0xFFFFFFFF;
        uint32_t copy_family_idx = 0xFFFFFFFF;
        {
            MTR_SCOPE("VulkanNovaRenderer", "Select Physical Device");
            for(uint32_t device_idx = 0; device_idx < device_count; device_idx++) {
                graphics_family_idx = 0xFFFFFFFF;
                VkPhysicalDevice current_device = physical_devices[device_idx];
                vkGetPhysicalDeviceProperties(current_device, &gpu.props);

                if(gpu.props.apiVersion < REQUIRED_VULKAN_VERSION) {
                    continue;
                }

                const bool is_intel_gpu = gpu.props.vendorID == INTEL_PCI_VENDOR_ID;
                const bool more_gpus_available = device_count - 1 > device_idx;
                if(is_intel_gpu && more_gpus_available) {
                    // Intel GPU _probably_ isn't as powerful as a discreet GPU, and if there's more than one GPU then the other one(s) are
                    // _probably_ discreet GPUs, so let's not use the Intel GPU and instead use the discreet GPU
                    // TODO: Make a local device for the integrated GPU when we figure out multi-GPU
                    // TODO: Rework this code when Intel releases discreet GPUs
                    continue;
                }

                // if(!is_intel_gpu) {
                // continue;
                // }

                const auto supports_extensions = does_device_support_extensions(current_device, device_extensions);
                if(!supports_extensions) {
                    continue;
                }

                uint32_t queue_family_count;
                vkGetPhysicalDeviceQueueFamilyProperties(current_device, &queue_family_count, nullptr);
                gpu.queue_family_props.resize(queue_family_count);
                vkGetPhysicalDeviceQueueFamilyProperties(current_device, &queue_family_count, gpu.queue_family_props.data());

                for(uint32_t queue_idx = 0; queue_idx < queue_family_count; queue_idx++) {
                    const VkQueueFamilyProperties current_properties = gpu.queue_family_props[queue_idx];
                    if(current_properties.queueCount < 1) {
                        continue;
                    }

                    VkBool32 supports_present = VK_FALSE;
                    NOVA_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(current_device, queue_idx, surface, &supports_present));
                    const VkQueueFlags supports_graphics = current_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT;
                    if((supports_graphics != 0U) && supports_present == VK_TRUE && graphics_family_idx == 0xFFFFFFFF) {
                        graphics_family_idx = queue_idx;
                    }

                    const VkQueueFlags supports_compute = current_properties.queueFlags & VK_QUEUE_COMPUTE_BIT;
                    if((supports_compute != 0U) && compute_family_idx == 0xFFFFFFFF) {
                        compute_family_idx = queue_idx;
                    }

                    const VkQueueFlags supports_copy = current_properties.queueFlags & VK_QUEUE_TRANSFER_BIT;
                    if((supports_copy != 0U) && copy_family_idx == 0xFFFFFFFF) {
                        copy_family_idx = queue_idx;
                    }
                }

                if(graphics_family_idx != 0xFFFFFFFF) {
                    logger->info("Selected GPU %s", gpu.props.deviceName);

                    has_nv_device_checkpoints = does_device_support_extensions(current_device,
                                                                               rx::array{
                                                                                   VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME});

                    gpu.phys_device = current_device;
                    break;
                }
            }
        }

        if(gpu.phys_device == nullptr) {
            logger->error("Failed to find good GPU");

            // TODO: Message the user that GPU selection failed

            return;
        }

        PROFILE_VOID_EXPR(vkGetPhysicalDeviceFeatures(gpu.phys_device, &gpu.supported_features),
                          VulkanRenderDevice,
                          vkGetPhysicalDeviceFeatures);

        PROFILE_VOID_EXPR(vkGetPhysicalDeviceMemoryProperties(gpu.phys_device, &gpu.memory_properties),
                          VulkanRenderDevice,
                          vkGetPhysicalDeviceMemoryProperties);

        const float priority = 1.0;

        VkDeviceQueueCreateInfo graphics_queue_create_info{};
        graphics_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphics_queue_create_info.pNext = nullptr;
        graphics_queue_create_info.flags = 0;
        graphics_queue_create_info.queueCount = 1;
        graphics_queue_create_info.queueFamilyIndex = graphics_family_idx;
        graphics_queue_create_info.pQueuePriorities = &priority;

        rx::vector<VkDeviceQueueCreateInfo> queue_create_infos{internal_allocator};
        queue_create_infos.push_back(graphics_queue_create_info);

        VkPhysicalDeviceFeatures physical_device_features{};
        physical_device_features.geometryShader = VK_TRUE;
        physical_device_features.tessellationShader = VK_TRUE;
        physical_device_features.samplerAnisotropy = VK_TRUE;
        physical_device_features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;

        if(settings->debug.enable_gpu_based_validation) {
            physical_device_features.fragmentStoresAndAtomics = VK_TRUE;
            physical_device_features.vertexPipelineStoresAndAtomics = VK_TRUE;
        }

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pNext = nullptr;
        device_create_info.flags = 0;
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.pEnabledFeatures = &physical_device_features;

        if(has_nv_device_checkpoints) {
            device_extensions.push_back(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
        }

        device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
        device_create_info.ppEnabledExtensionNames = device_extensions.data();
        device_create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layer_names.size());
        if(!enabled_layer_names.is_empty()) {
            device_create_info.ppEnabledLayerNames = enabled_layer_names.data();
        }

        // Set up descriptor indexing
        // Currently Nova only cares about indexing for texture descriptors
        VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features = {};
        descriptor_indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        descriptor_indexing_features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        descriptor_indexing_features.runtimeDescriptorArray = true;
        descriptor_indexing_features.descriptorBindingVariableDescriptorCount = VK_TRUE;
        descriptor_indexing_features.descriptorBindingPartiallyBound = VK_TRUE;
        descriptor_indexing_features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        device_create_info.pNext = &descriptor_indexing_features;

        VkDevice vk_device;
        const auto res = PROFILE_RET_EXPR(vkCreateDevice(gpu.phys_device, &device_create_info, nullptr, &vk_device),
                                          VulkanRenderEngine,
                                          vkCreateDevice);
        if(res != VK_SUCCESS) {
            logger->error("Could not create Vulkan device: %s", vk::to_string(vk::Result{res}).c_str());
            rx::log::flush();
        }
        device = vk_device;

        device_dynamic_loader.init(instance, vkGetInstanceProcAddr, device);

        graphics_family_index = graphics_family_idx;
        vkGetDeviceQueue(device, graphics_family_idx, 0, &graphics_queue);
        compute_family_index = compute_family_idx;
        vkGetDeviceQueue(device, compute_family_idx, 0, &compute_queue);
        transfer_family_index = copy_family_idx;
        vkGetDeviceQueue(device, copy_family_idx, 0, &copy_queue);
    }

    bool VulkanRenderDevice::does_device_support_extensions(VkPhysicalDevice device,
                                                            const rx::vector<const char*>& required_device_extensions) {
        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
        rx::vector<VkExtensionProperties> available(extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available.data());

        rx::set<rx::string> required{internal_allocator};
        required_device_extensions.each_fwd([&](const char* extension) { required.insert(extension); });

        available.each_fwd(
            [&](const VkExtensionProperties& extension) { required.erase(static_cast<const char*>(extension.extensionName)); });

        if(!required.is_empty()) {
            std::stringstream ss;
            required.each([&](const rx::string& extension) { ss << extension.data() << ", "; });

            logger->warning("Device does not support these required extensions: %s", ss.str().c_str());
        }

        const auto device_supports_required_extensions = required.is_empty();
        return device_supports_required_extensions;
    }

    void VulkanRenderDevice::create_swapchain() {
        MTR_SCOPE("VulkanRenderDevice", "create_swapchain");
        // Check what formats our rendering supports, and create a swapchain with one of those formats

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.phys_device, surface, &gpu.surface_capabilities);

        uint32_t num_surface_formats;
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.phys_device, surface, &num_surface_formats, nullptr);
        gpu.surface_formats.resize(num_surface_formats);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.phys_device, surface, &num_surface_formats, gpu.surface_formats.data());

        uint32_t num_surface_present_modes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.phys_device, surface, &num_surface_present_modes, nullptr);
        rx::vector<VkPresentModeKHR> present_modes{internal_allocator, num_surface_present_modes};
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.phys_device, surface, &num_surface_present_modes, present_modes.data());

        swapchain = rx::make_ptr<VulkanSwapchain>(internal_allocator,
                                                  settings->max_in_flight_frames,
                                                  this,
                                                  window.get_framebuffer_size(),
                                                  present_modes);

        swapchain_size = window.get_framebuffer_size();
    }

    void VulkanRenderDevice::create_per_thread_command_pools() {
        MTR_SCOPE("VulkanRenderDevice", "create_per_thread_command_pools");
        const uint32_t num_threads = 1; // TODO: Make this real
        command_pools_by_thread_idx.reserve(num_threads);

        for(uint32_t i = 0; i < num_threads; i++) {
            command_pools_by_thread_idx.push_back(make_new_command_pools());
        }
    }

    void VulkanRenderDevice::create_standard_pipeline_layout() {
        standard_push_constants = rx::array{
            // Camera and Material index
            vk::PushConstantRange().setStageFlags(vk::ShaderStageFlagBits::eAll).setOffset(0).setSize(sizeof(uint32_t) * 2)};

        const auto flags_per_binding = rx::array{vk::DescriptorBindingFlags{},
                                                 vk::DescriptorBindingFlags{},
                                                 vk::DescriptorBindingFlags{},
                                                 vk::DescriptorBindingFlags{},
                                                 vk::DescriptorBindingFlags{},
                                                 vk::DescriptorBindingFlagBits::eUpdateAfterBind |
                                                     vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
                                                     vk::DescriptorBindingFlagBits::ePartiallyBound};

        const auto set_flags = vk::DescriptorSetLayoutBindingFlagsCreateInfo()
                                   .setBindingCount(static_cast<uint32_t>(flags_per_binding.size()))
                                   .setPBindingFlags(flags_per_binding.data());

        const auto camera_buffer_descriptor_type = (MAX_NUM_CAMERAS * sizeof(CameraUboData)) < gpu.props.limits.maxUniformBufferRange ?
                                                       vk::DescriptorType::eUniformBuffer :
                                                       vk::DescriptorType::eStorageBuffer;

        const auto material_buffer_descriptor_type = MATERIAL_BUFFER_SIZE.b_count() < gpu.props.limits.maxUniformBufferRange ?
                                                         vk::DescriptorType::eUniformBuffer :
                                                         vk::DescriptorType::eStorageBuffer;

        // Binding for the array of material parameter buffers. Nova uses a variable-length, partially-bound
        const rx::vector<vk::DescriptorSetLayoutBinding> bindings = rx::array{// Camera data buffer
                                                                              vk::DescriptorSetLayoutBinding()
                                                                                  .setBinding(0)
                                                                                  .setDescriptorType(camera_buffer_descriptor_type)
                                                                                  .setDescriptorCount(1)
                                                                                  .setStageFlags(vk::ShaderStageFlagBits::eAll),
                                                                              // Material data buffer
                                                                              vk::DescriptorSetLayoutBinding()
                                                                                  .setBinding(1)
                                                                                  .setDescriptorType(material_buffer_descriptor_type)
                                                                                  .setDescriptorCount(1)
                                                                                  .setStageFlags(vk::ShaderStageFlagBits::eAll),
                                                                              // Point sampler
                                                                              vk::DescriptorSetLayoutBinding()
                                                                                  .setBinding(2)
                                                                                  .setDescriptorType(vk::DescriptorType::eSampler)
                                                                                  .setDescriptorCount(1)
                                                                                  .setStageFlags(vk::ShaderStageFlagBits::eAll),
                                                                              // Bilinear sampler
                                                                              vk::DescriptorSetLayoutBinding()
                                                                                  .setBinding(3)
                                                                                  .setDescriptorType(vk::DescriptorType::eSampler)
                                                                                  .setDescriptorCount(1)
                                                                                  .setStageFlags(vk::ShaderStageFlagBits::eAll),
                                                                              // Trilinear sampler
                                                                              vk::DescriptorSetLayoutBinding()
                                                                                  .setBinding(4)
                                                                                  .setDescriptorType(vk::DescriptorType::eSampler)
                                                                                  .setDescriptorCount(1)
                                                                                  .setStageFlags(vk::ShaderStageFlagBits::eAll),
                                                                              // Textures array
                                                                              vk::DescriptorSetLayoutBinding()
                                                                                  .setBinding(5)
                                                                                  .setDescriptorType(vk::DescriptorType::eSampledImage)
                                                                                  .setDescriptorCount(MAX_NUM_TEXTURES)
                                                                                  .setStageFlags(vk::ShaderStageFlagBits::eAll)};

        const auto dsl_layout_create = vk::DescriptorSetLayoutCreateInfo()
                                           .setFlags(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool)
                                           .setBindingCount(static_cast<uint32_t>(bindings.size()))
                                           .setPBindings(bindings.data())
                                           .setPNext(&set_flags);

        device.createDescriptorSetLayout(&dsl_layout_create, nullptr, &standard_set_layout);

        const auto pipeline_layout_create = vk::PipelineLayoutCreateInfo()
                                                .setSetLayoutCount(1)
                                                .setPSetLayouts(&standard_set_layout)
                                                .setPushConstantRangeCount(static_cast<uint32_t>(standard_push_constants.size()))
                                                .setPPushConstantRanges(standard_push_constants.data());

        device.createPipelineLayout(&pipeline_layout_create, nullptr, &standard_pipeline_layout);

        const auto& pool = create_descriptor_pool(rx::array{rx::pair{DescriptorType::StorageBuffer, 5_u32 * 128},
                                                            rx::pair{DescriptorType::UniformBuffer, 5_u32 * 128},
                                                            rx::pair{DescriptorType::Texture, MAX_NUM_TEXTURES * 128},
                                                            rx::pair{DescriptorType::Sampler, 3_u32 * 128}},
                                                  internal_allocator);

        standard_descriptor_set_pool = *pool;

        if(settings->debug.enabled) {
            const auto pipeline_layout_name = vk::DebugUtilsObjectNameInfoEXT{}
                                                  .setObjectType(vk::ObjectType::ePipelineLayout)
                                                  .setObjectHandle(
                                                      reinterpret_cast<uint64_t>(static_cast<VkPipelineLayout>(standard_pipeline_layout)))
                                                  .setPObjectName("Standard Pipeline Layout");
            device.setDebugUtilsObjectNameEXT(&pipeline_layout_name, device_dynamic_loader);

            const auto descriptor_pool_name = vk::DebugUtilsObjectNameInfoEXT{}
                                                  .setObjectType(vk::ObjectType::eDescriptorPool)
                                                  .setObjectHandle(reinterpret_cast<uint64_t>(
                                                      static_cast<VkDescriptorPool>(standard_descriptor_set_pool)))
                                                  .setPObjectName("Standard Descriptor Set Pool");
            device.setDebugUtilsObjectNameEXT(&descriptor_pool_name, device_dynamic_loader);

            const auto descriptor_set_layout_name = vk::DebugUtilsObjectNameInfoEXT{}
                                                        .setObjectType(vk::ObjectType::eDescriptorSetLayout)
                                                        .setObjectHandle(reinterpret_cast<uint64_t>(
                                                            static_cast<VkDescriptorSetLayout>(standard_set_layout)))
                                                        .setPObjectName("Standard Descriptor Set Layout");
            device.setDebugUtilsObjectNameEXT(&descriptor_set_layout_name, device_dynamic_loader);
        }
    }

    void VulkanRenderDevice::create_material_resource_binder() {
        standard_layout_bindings.insert("cameras",
                                        RhiResourceBindingDescription{/* .set = */ 0,
                                                                      /* .binding = */ 0,
                                                                      /* .count = */ 1,
                                                                      /* .is_unbounded = */ false,
                                                                      /* .type = */ DescriptorType::StorageBuffer,
                                                                      /* .stages = */ ShaderStage::All});

        standard_layout_bindings.insert("material_buffer",
                                        RhiResourceBindingDescription{/* .set = */ 0,
                                                                      /* .binding = */ 1,
                                                                      /* .count = */ 1,
                                                                      /* .is_unbounded = */ false,
                                                                      /* .type = */ DescriptorType::StorageBuffer,
                                                                      /* .stages = */ ShaderStage::All});

        standard_layout_bindings.insert("point_sampler",
                                        RhiResourceBindingDescription{/* .set = */ 0,
                                                                      /* .binding = */ 2,
                                                                      /* .count = */ 1,
                                                                      /* .is_unbounded = */ false,
                                                                      /* .type = */ DescriptorType::Sampler,
                                                                      /* .stages = */ ShaderStage::All});

        standard_layout_bindings.insert("bilinear_sampler",
                                        RhiResourceBindingDescription{/* .set = */ 0,
                                                                      /* .binding = */ 3,
                                                                      /* .count = */ 1,
                                                                      /* .is_unbounded = */ false,
                                                                      /* .type = */ DescriptorType::Sampler,
                                                                      /* .stages = */ ShaderStage::All});

        standard_layout_bindings.insert("trilinear_sampler",
                                        RhiResourceBindingDescription{/* .set = */ 0,
                                                                      /* .binding = */ 4,
                                                                      /* .count = */ 1,
                                                                      /* .is_unbounded = */ false,
                                                                      /* .type = */ DescriptorType::Sampler,
                                                                      /* .stages = */ ShaderStage::All});

        standard_layout_bindings.insert("textures",
                                        RhiResourceBindingDescription{/* .set = */ 0,
                                                                      /* .binding = */ 5,
                                                                      /* .count = */ MAX_NUM_TEXTURES,
                                                                      /* .is_unbounded = */ true,
                                                                      /* .type = */ DescriptorType::Texture,
                                                                      /* .stages = */ ShaderStage::All});

        auto set = get_next_standard_descriptor_set();

        material_resource_binder = rx::make_ptr<VulkanResourceBinder>(internal_allocator,
                                                                      *this,
                                                                      standard_layout_bindings,
                                                                      rx::array{set},
                                                                      standard_pipeline_layout,
                                                                      internal_allocator);
    }

    rx::map<uint32_t, VkCommandPool> VulkanRenderDevice::make_new_command_pools() const {
        MTR_SCOPE("VulkanRenderDevice", "make_new_command_pools");
        rx::vector<uint32_t> queue_indices{internal_allocator};
        queue_indices.push_back(graphics_family_index);
        queue_indices.push_back(transfer_family_index);
        queue_indices.push_back(compute_family_index);

        rx::map<uint32_t, VkCommandPool> pools_by_queue{internal_allocator};

        queue_indices.each_fwd([&](const uint32_t queue_index) {
            VkCommandPoolCreateInfo command_pool_create_info;
            command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            command_pool_create_info.pNext = nullptr;
            command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            command_pool_create_info.queueFamilyIndex = queue_index;

            VkCommandPool command_pool;
            NOVA_CHECK_RESULT(vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool));
            pools_by_queue.insert(queue_index, command_pool);
        });

        return pools_by_queue;
    }

    uint32_t VulkanRenderDevice::find_memory_type_with_flags(const uint32_t search_flags, const MemorySearchMode search_mode) const {
        for(uint32_t i = 0; i < gpu.memory_properties.memoryTypeCount; i++) {
            const VkMemoryType& memory_type = gpu.memory_properties.memoryTypes[i];
            switch(search_mode) {
                case MemorySearchMode::Exact:
                    if(memory_type.propertyFlags == search_flags) {
                        return i;
                    }
                    break;

                case MemorySearchMode::Fuzzy:
                    if((memory_type.propertyFlags & search_flags) != 0) {
                        return i;
                    }
                    break;
            }
        }

        return VK_MAX_MEMORY_TYPES;
    }

    VkImageView VulkanRenderDevice::image_view_for_image(const RhiImage* image) {
        // TODO: This method is terrible. We shouldn't tie image views to images, we should let everything that wants
        // to use the image create its own image view

        const auto* vk_image = static_cast<const VulkanImage*>(image);

        return vk_image->image_view;
    }

    VkCommandBufferLevel VulkanRenderDevice::to_vk_command_buffer_level(const RhiRenderCommandList::Level level) {
        switch(level) {
            case RhiRenderCommandList::Level::Primary:
                return VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            case RhiRenderCommandList::Level::Secondary:
                return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        }

        return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    }

    VulkanInputAssemblerLayout VulkanRenderDevice::get_input_assembler_setup(const rx::vector<RhiVertexField>& vertex_fields) {
        rx::vector<VkVertexInputAttributeDescription> attributes;
        rx::vector<VkVertexInputBindingDescription> bindings;

        attributes.reserve(vertex_fields.size());
        bindings.reserve(vertex_fields.size());

        uint32_t vertex_size = 0;
        vertex_fields.each_fwd([&](const RhiVertexField& field) { vertex_size += get_byte_size(field.format); });

        uint32_t cur_binding = 0;
        uint32_t byte_offset = 0;
        vertex_fields.each_fwd([&](const RhiVertexField& field) {
            const auto field_size = get_byte_size(field.format);
            const auto attr_format = to_vk_vertex_format(field.format);
            attributes.emplace_back(VkVertexInputAttributeDescription{cur_binding, 0, attr_format, byte_offset});

            bindings.emplace_back(VkVertexInputBindingDescription{cur_binding, vertex_size, VK_VERTEX_INPUT_RATE_VERTEX});

            cur_binding++;
            byte_offset += field_size;
        });

        return {attributes, bindings};
    }

    rx::optional<VkShaderModule> VulkanRenderDevice::create_shader_module(const rx::vector<uint32_t>& spirv) const {
        MTR_SCOPE("VulkanRenderDevice", "create_shader_module");
        VkShaderModuleCreateInfo shader_module_create_info = {};
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.pCode = spirv.data();
        shader_module_create_info.codeSize = spirv.size() * 4;

        VkShaderModule module;
        const auto result = vkCreateShaderModule(device, &shader_module_create_info, nullptr, &module);
        if(result == VK_SUCCESS) {
            return rx::optional<VkShaderModule>(module);

        } else {
            logger->error("Could not create shader module: %s", to_string(result));
            return rx::nullopt;
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderDevice::debug_report_callback(const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                                             const VkDebugUtilsMessageTypeFlagsEXT message_types,
                                                                             const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                                             void* render_device) {
        rx::string type = "General";
        if((message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0U) {
            type = "Validation";
        } else if((message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0U) {
            type = "Performance";
        }

        rx::string queue_list;
        if(callback_data->queueLabelCount != 0) {
            queue_list.append(" Queues: ");
            for(uint32_t i = 0; i < callback_data->queueLabelCount; i++) {
                queue_list.append(callback_data->pQueueLabels[i].pLabelName);
                if(i != callback_data->queueLabelCount - 1) {
                    queue_list.append(", ");
                }
            }
        }

        rx::string command_buffer_list;
        if(callback_data->cmdBufLabelCount != 0) {
            command_buffer_list.append("Command Buffers: ");
            for(uint32_t i = 0; i < callback_data->cmdBufLabelCount; i++) {
                command_buffer_list.append(callback_data->pCmdBufLabels[i].pLabelName);
                if(i != callback_data->cmdBufLabelCount - 1) {
                    command_buffer_list.append(", ");
                }
            }
        }

        rx::string object_list;
        if(callback_data->objectCount != 0) {
            object_list.append("Objects: ");
            for(uint32_t i = 0; i < callback_data->objectCount; i++) {
                object_list.append(to_string(callback_data->pObjects[i].objectType));
                if(callback_data->pObjects[i].pObjectName != nullptr) {
                    object_list.append(rx::string::format(" \"%s\"", callback_data->pObjects[i].pObjectName));
                }
                object_list.append(rx::string::format(" (%x)", callback_data->pObjects[i].objectHandle));
                if(i != callback_data->objectCount - 1) {
                    object_list.append(", ");
                }
            }
        }

        rx::string vk_message;
        if(callback_data->pMessage != nullptr) {
            vk_message.append(callback_data->pMessage);
        }

        const rx::string msg = rx::string::format("[%s] %s %s %s %s", type, queue_list, command_buffer_list, object_list, vk_message);

        if((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
            logger->error("%s", msg);
#ifdef NOVA_LINUX
            nova_backtrace();
#endif

            auto* vk_render_device = reinterpret_cast<VulkanRenderDevice*>(render_device);
            if(vk_render_device->settings->debug.break_on_validation_errors) {
                rx::abort("Vulkan validation error");
            }

        } else if((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
            // Warnings may hint at unexpected / non-spec API usage
            logger->warning("%s", msg);

        } else if(((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) &&
                  ((message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) == 0U)) { // No validation info!
            // Informal messages that may become handy during debugging
            logger->info("%s", msg);

        } else if((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) != 0) {
            // Diagnostic info from the Vulkan loader and layers
            // Usually not helpful in terms of API usage, but may help to debug layer and loader problems
            logger->verbose("%s", msg);

        } else if((message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) == 0U) { // No validation info!
            // Catch-all to be super sure
            logger->info("%s", msg);
        }

        return VK_FALSE;
    }
} // namespace nova::renderer::rhi
