/*!
 * \author ddubois
 * \date 06-Feb-19.
 */

#include "../../../tests/src/general_test_setup.hpp"
#include "../../loading/shaderpack/render_graph_builder.hpp"
#include "../../loading/shaderpack/shaderpack_loading.hpp"
#include "swapchain.hpp"
#include "vulkan_render_engine.hpp"
#include "vulkan_type_converters.hpp"
#include "vulkan_utils.hpp"

namespace nova::renderer {
    enum class barrier_necessity { maybe, yes, no };

    void vulkan_render_engine::set_shaderpack(const shaderpack_data& data) {
        NOVA_LOG(DEBUG) << "Vulkan render engine loading new shaderpack";
        if(shaderpack_loaded) {
            destroy_render_passes();
            destroy_graphics_pipelines();
            materials.clear();
            material_passes_by_pipeline.clear();
            destroy_dynamic_resources();

            NOVA_LOG(DEBUG) << "Resources from old shaderpacks destroyed";
        }

        create_textures(data.resources.textures);
        NOVA_LOG(DEBUG) << "Dynamic textures created";
        for(const material_data& mat_data : data.materials) {
            materials[mat_data.name] = mat_data;

            for(const material_pass& mat : mat_data.passes) {
                material_passes_by_pipeline[mat.pipeline].push_back(mat);
            }
        }
        NOVA_LOG(DEBUG) << "Materials saved";

        create_render_passes(data.passes);
        NOVA_LOG(DEBUG) << "Created render passes";
        create_graphics_pipelines(data.pipelines);
        NOVA_LOG(DEBUG) << "Created pipelines";

        create_material_descriptor_sets();
        NOVA_LOG(TRACE) << "Material descriptor sets created";

        generate_barriers_for_dynamic_resources();

        shaderpack_loaded = true;
    }

    void vulkan_render_engine::create_textures(const std::vector<texture_resource_data>& texture_datas) {
        const VkExtent2D swapchain_extent = swapchain->get_swapchain_extent();
        const glm::uvec2 swapchain_extent_glm = {swapchain_extent.width, swapchain_extent.height};
        for(const texture_resource_data& texture_data : texture_datas) {
            vk_texture texture;
            texture.is_dynamic = true;
            texture.data = texture_data;
            texture.format = to_vk_format(texture_data.format.pixel_format);
            texture.layout = VK_IMAGE_LAYOUT_UNDEFINED;

            VkImageCreateInfo image_create_info = {};
            image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.format = texture.format;
            const glm::uvec2 texture_size = texture_data.format.get_size_in_pixels(swapchain_extent_glm);
            image_create_info.extent.width = texture_size.x;
            image_create_info.extent.height = texture_size.y;
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
            if(texture.format == VK_FORMAT_D24_UNORM_S8_UINT || texture.format == VK_FORMAT_D32_SFLOAT) {
                image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            } else {
                image_create_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
            image_create_info.queueFamilyIndexCount = 1;
            image_create_info.pQueueFamilyIndices = &graphics_family_index;
            image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            VmaAllocationCreateInfo alloc_create_info = {};
            alloc_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            alloc_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            alloc_create_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            alloc_create_info.preferredFlags = 0;
            alloc_create_info.memoryTypeBits = 0;
            alloc_create_info.pool = nullptr;
            alloc_create_info.pUserData = nullptr;

            vmaCreateImage(vma_allocator, &image_create_info, &alloc_create_info, &texture.image, &texture.allocation, &texture.vma_info);

            VkImageViewCreateInfo image_view_create_info = {};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.image = texture.image;
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = image_create_info.format;
            if(texture.format == VK_FORMAT_D24_UNORM_S8_UINT || texture.format == VK_FORMAT_D32_SFLOAT) {
                image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                texture.is_depth_tex = true;
            } else {
                image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;

            vkCreateImageView(device, &image_view_create_info, nullptr, &texture.image_view);

            if(settings.debug.enabled) {
                VkDebugUtilsObjectNameInfoEXT object_name = {};
                object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                object_name.objectType = VK_OBJECT_TYPE_IMAGE;
                object_name.objectHandle = reinterpret_cast<uint64_t>(texture.image);
                object_name.pObjectName = texture_data.name.c_str();
                NOVA_THROW_IF_VK_ERROR(vkSetDebugUtilsObjectNameEXT(device, &object_name), render_engine_initialization_exception);
                NOVA_LOG(INFO) << "Set object " << texture.image << " to have name " << texture_data.name;
            }

            textures[texture_data.name] = texture;
        }

        dynamic_textures_need_to_transition = true;
    }

    void vulkan_render_engine::create_render_passes(const std::vector<render_pass_data>& passes) {
        NOVA_LOG(DEBUG) << "Flattening frame graph...";

        std::unordered_map<std::string, render_pass_data> regular_render_passes;
        regular_render_passes.reserve(passes.size());
        render_passes.reserve(passes.size());
        for(const render_pass_data& pass_data : passes) {
            render_passes[pass_data.name].data = pass_data;
            VkFenceCreateInfo fence_info = {};
            fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            vkCreateFence(device, &fence_info, nullptr, &render_passes[pass_data.name].fence);
            regular_render_passes[pass_data.name] = pass_data;
        }

        render_passes_by_order = order_passes(regular_render_passes);

        const VkExtent2D swapchain_extent = swapchain->get_swapchain_extent();

        for(const auto& [pass_name, pass] : render_passes) {
            VkSubpassDescription subpass_description;
            subpass_description.flags = 0;
            subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass_description.inputAttachmentCount = 0;
            subpass_description.pInputAttachments = nullptr;
            subpass_description.preserveAttachmentCount = 0;
            subpass_description.pPreserveAttachments = nullptr;
            subpass_description.pResolveAttachments = nullptr;
            subpass_description.pDepthStencilAttachment = nullptr;

            VkSubpassDependency image_available_dependency;
            image_available_dependency.dependencyFlags = 0;
            image_available_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            image_available_dependency.dstSubpass = 0;
            image_available_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            image_available_dependency.srcAccessMask = 0;
            image_available_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            image_available_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            VkRenderPassCreateInfo render_pass_create_info;
            render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            render_pass_create_info.pNext = nullptr;
            render_pass_create_info.flags = 0;
            render_pass_create_info.subpassCount = 1;
            render_pass_create_info.pSubpasses = &subpass_description;
            render_pass_create_info.dependencyCount = 1;
            render_pass_create_info.pDependencies = &image_available_dependency;

            std::vector<VkAttachmentReference> attachment_references;
            std::vector<VkAttachmentDescription> attachments;
            std::vector<VkImageView> framebuffer_attachments;
            uint32_t framebuffer_width = 0;
            uint32_t framebuffer_height = 0;

            std::vector<vk_texture*> textures_in_framebuffer;
            textures_in_framebuffer.reserve(pass.data.texture_outputs.size() + (pass.data.depth_texture ? 1 : 0));
            bool writes_to_backbuffer = false;
            // Collect framebuffer size information from color output attachments
            for(const texture_attachment& attachment : pass.data.texture_outputs) {
                if(attachment.name == "Backbuffer") {
                    // Handle backbuffer
                    // Backbuffer framebuffers are handled by themselves in their own special snowflake way so we just need to skip
                    // everything
                    writes_to_backbuffer = true;

                    VkAttachmentDescription desc = {};
                    desc.flags = 0;
                    desc.format = swapchain->get_swapchain_format();
                    desc.samples = VK_SAMPLE_COUNT_1_BIT;
                    desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    attachments.push_back(desc);

                    VkAttachmentReference ref = {};

                    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    ref.attachment = static_cast<uint32_t>(attachments.size()) - 1;

                    attachment_references.push_back(ref);

                    framebuffer_width = swapchain_extent.width;
                    framebuffer_height = swapchain_extent.height;

                    render_passes[pass_name].writes_to_backbuffer = true;

                    break;
                }
                const vk_texture& tex = textures.at(attachment.name);
                // the textures array _should_ _never_ change from this point onward. If it does, this pointer will probably point to
                // undefined data
                textures_in_framebuffer.push_back(&textures.at(attachment.name));

                framebuffer_attachments.push_back(tex.image_view);

                const glm::uvec2 attachment_size = tex.data.format.get_size_in_pixels({swapchain_extent.width, swapchain_extent.height});

                if(framebuffer_width == 0) {
                    framebuffer_width = attachment_size.x;
                } else if(attachment_size.x != framebuffer_width) {
                    NOVA_LOG(ERROR) << "Texture " << attachment.name << " used by renderpass " << pass_name << " has a width of "
                                    << attachment_size.x << ", but the framebuffer has a width of " << framebuffer_width
                                    << ". This is illegal, all input textures of a single renderpass must be the same size";
                }

                if(framebuffer_height == 0) {
                    framebuffer_height = attachment_size.y;
                } else if(attachment_size.y != framebuffer_height) {
                    NOVA_LOG(ERROR) << "Texture " << attachment.name << " used by renderpass " << pass_name << " has a height of "
                                    << attachment_size.y << ", but the framebuffer has a height of " << framebuffer_height
                                    << ". This is illegal, all input textures of a single renderpass must be the same size";
                }

                NOVA_LOG(TRACE) << "Adding image view " << textures.at(attachment.name).image_view << " from image " << attachment.name
                                << " to framebuffer for renderpass " << pass.data.name;

                VkAttachmentDescription desc = {};
                desc.flags = 0;
                desc.format = tex.format;
                desc.samples = VK_SAMPLE_COUNT_1_BIT;
                desc.loadOp = attachment.clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
                desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                attachments.push_back(desc);

                VkAttachmentReference ref = {};

                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                ref.attachment = static_cast<uint32_t>(attachments.size()) - 1;

                attachment_references.push_back(ref);
            }

            VkAttachmentReference depth_reference = {};
            // Collect framebuffer size information from the depth attachment
            if(pass.data.depth_texture) {

                const vk_texture& tex = textures.at(pass.data.depth_texture->name);
                framebuffer_attachments.push_back(tex.image_view);

                textures_in_framebuffer.push_back(&textures.at(pass.data.depth_texture->name));

                const glm::uvec2 attachment_size = tex.data.format.get_size_in_pixels({swapchain_extent.width, swapchain_extent.height});

                if(framebuffer_width == 0) {
                    framebuffer_width = attachment_size.x;
                } else if(attachment_size.x != framebuffer_width) {
                    NOVA_LOG(ERROR) << "Texture " << pass.data.depth_texture->name << " used by renderpass " << pass_name
                                    << " has a width of " << attachment_size.x << ", but the framebuffer has a width of "
                                    << framebuffer_width
                                    << ". This is illegal, all input textures of a single renderpass must be the same size";
                }

                if(framebuffer_height == 0) {
                    framebuffer_height = attachment_size.y;
                } else if(attachment_size.y != framebuffer_height) {
                    NOVA_LOG(ERROR) << "Texture " << pass.data.depth_texture->name << " used by renderpass " << pass_name
                                    << " has a height of " << attachment_size.y << ", but the framebuffer has a height of "
                                    << framebuffer_height
                                    << ". This is illegal, all input textures of a single renderpass must be the same size";
                }

                VkAttachmentDescription desc = {};
                desc.flags = 0;
                desc.format = tex.format;
                desc.samples = VK_SAMPLE_COUNT_1_BIT;
                desc.loadOp = pass.data.depth_texture->clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
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
                NOVA_LOG(ERROR)
                    << "Framebuffer width for pass " << pass.data.name
                    << " is 0. This is illegal! Make sure that there is at least one attachment for this render pass, and ensure that all attachments used by this pass have a non-zero width";
            }

            if(framebuffer_height == 0) {
                NOVA_LOG(ERROR)
                    << "Framebuffer height for pass " << pass.data.name
                    << " is 0. This is illegal! Make sure that there is at least one attachment for this render pass, and ensure that all attachments used by this pass have a non-zero height";
            }

            if(framebuffer_attachments.size() > gpu.props.limits.maxColorAttachments) {
                NOVA_LOG(ERROR)
                    << "Framebuffer for pass " << pass.data.name << " has " << framebuffer_attachments.size()
                    << " color attachments, but your GPU only supports " << gpu.props.limits.maxColorAttachments
                    << ". Please reduce the number of attachments that this pass uses, possibly by changing some of your input attachments to bound textures";
            }

            subpass_description.colorAttachmentCount = static_cast<uint32_t>(attachment_references.size());
            subpass_description.pColorAttachments = attachment_references.data();

            render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
            render_pass_create_info.pAttachments = attachments.data();

            NOVA_THROW_IF_VK_ERROR(vkCreateRenderPass(device, &render_pass_create_info, nullptr, &render_passes[pass_name].pass),
                                   render_engine_initialization_exception);

            if(writes_to_backbuffer) {
                if(pass.data.texture_outputs.size() > 1) {
                    NOVA_LOG(ERROR)
                        << "Pass " << pass.data.name
                        << " writes to the backbuffer, and other textures. Passes that write to the backbuffer are not allowed to write to any other textures";
                }

                render_passes[pass_name].framebuffer.framebuffer = nullptr;
            } else {
                VkFramebufferCreateInfo framebuffer_create_info = {};
                framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebuffer_create_info.renderPass = render_passes[pass_name].pass;
                framebuffer_create_info.attachmentCount = static_cast<uint32_t>(framebuffer_attachments.size());
                framebuffer_create_info.pAttachments = framebuffer_attachments.data();
                framebuffer_create_info.width = framebuffer_width;
                framebuffer_create_info.height = framebuffer_height;
                framebuffer_create_info.layers = 1;

                std::stringstream ss;
                for(const VkImageView& attachment : framebuffer_attachments) {
                    ss << attachment << ", ";
                }

                NOVA_LOG(TRACE) << "Creating framebuffer with size (" << framebuffer_width << ", " << framebuffer_height
                                << "), and attachments " << ss.str();

                VkFramebuffer framebuffer;
                NOVA_THROW_IF_VK_ERROR(vkCreateFramebuffer(device, &framebuffer_create_info, nullptr, &framebuffer),
                                       render_engine_initialization_exception);
                render_passes[pass_name].framebuffer.framebuffer = framebuffer;
                render_passes[pass_name].framebuffer.images = std::move(textures_in_framebuffer);
            }

            render_passes[pass_name].render_area = {{0, 0}, {framebuffer_width, framebuffer_height}};

            if(settings.debug.enabled) {
                VkDebugUtilsObjectNameInfoEXT object_name = {};
                object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                object_name.objectType = VK_OBJECT_TYPE_IMAGE;
                object_name.objectHandle = reinterpret_cast<uint64_t>(render_passes[pass_name].pass);
                object_name.pObjectName = pass_name.c_str();
                NOVA_THROW_IF_VK_ERROR(vkSetDebugUtilsObjectNameEXT(device, &object_name), render_engine_initialization_exception);
            }
        }
    }

    void vulkan_render_engine::create_graphics_pipelines(const std::vector<pipeline_data>& pipelines) {
        const VkExtent2D swapchain_extent = swapchain->get_swapchain_extent();

        for(const pipeline_data& data : pipelines) {
            NOVA_LOG(TRACE) << "Creating a VkPipeline for pipeline " << data.name;
            vk_pipeline nova_pipeline;
            nova_pipeline.data = data;

            std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
            std::unordered_map<VkShaderStageFlags, VkShaderModule> shader_modules;

            NOVA_LOG(TRACE) << "Compiling vertex module";
            shader_modules[VK_SHADER_STAGE_VERTEX_BIT] = create_shader_module(data.vertex_shader.source);
            get_shader_module_descriptors(data.vertex_shader.source, VK_SHADER_STAGE_VERTEX_BIT, nova_pipeline.bindings);

            if(data.geometry_shader) {
                NOVA_LOG(TRACE) << "Compiling geometry module";
                shader_modules[VK_SHADER_STAGE_GEOMETRY_BIT] = create_shader_module(data.geometry_shader->source);
                get_shader_module_descriptors(data.geometry_shader->source, VK_SHADER_STAGE_GEOMETRY_BIT, nova_pipeline.bindings);
            }

            if(data.tessellation_control_shader) {
                NOVA_LOG(TRACE) << "Compiling tessellation_control module";
                shader_modules[VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT] = create_shader_module(data.tessellation_control_shader->source);
                get_shader_module_descriptors(data.tessellation_control_shader->source,
                                              VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                              nova_pipeline.bindings);
            }

            if(data.tessellation_evaluation_shader) {
                NOVA_LOG(TRACE) << "Compiling tessellation_evaluation module";
                shader_modules[VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT] = create_shader_module(
                    data.tessellation_evaluation_shader->source);
                get_shader_module_descriptors(data.tessellation_evaluation_shader->source,
                                              VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                              nova_pipeline.bindings);
            }

            if(data.fragment_shader) {
                NOVA_LOG(TRACE) << "Compiling fragment module";
                shader_modules[VK_SHADER_STAGE_FRAGMENT_BIT] = create_shader_module(data.fragment_shader->source);
                get_shader_module_descriptors(data.fragment_shader->source, VK_SHADER_STAGE_FRAGMENT_BIT, nova_pipeline.bindings);
            }

            nova_pipeline.layouts = create_descriptor_set_layouts(nova_pipeline.bindings);

            VkPipelineLayoutCreateInfo pipeline_layout_create_info;
            pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.pNext = nullptr;
            pipeline_layout_create_info.flags = 0;
            pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(nova_pipeline.layouts.size());
            pipeline_layout_create_info.pSetLayouts = nova_pipeline.layouts.data();
            pipeline_layout_create_info.pushConstantRangeCount = 0;
            pipeline_layout_create_info.pPushConstantRanges = nullptr;

            VkPipelineLayout layout;
            NOVA_THROW_IF_VK_ERROR(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &layout),
                                   render_engine_initialization_exception);
            nova_pipeline.layout = layout;

            for(const auto& [stage, shader_module] : shader_modules) {
                VkPipelineShaderStageCreateInfo shader_stage_create_info;
                shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shader_stage_create_info.pNext = nullptr;
                shader_stage_create_info.flags = 0;
                shader_stage_create_info.stage = static_cast<VkShaderStageFlagBits>(stage);
                shader_stage_create_info.module = shader_module;
                shader_stage_create_info.pName = "main";
                shader_stage_create_info.pSpecializationInfo = nullptr;

                shader_stages.push_back(shader_stage_create_info);
            }

            const std::vector<VkVertexInputBindingDescription>&
                vertex_binding_descriptions = vulkan::get_vertex_input_binding_descriptions();
            const std::vector<VkVertexInputAttributeDescription>&
                vertex_attribute_descriptions = vulkan::get_vertex_input_attribute_descriptions();

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
            switch(data.primitive_mode) {
                case primitive_topology_enum::Triangles:
                    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                    break;
                case primitive_topology_enum::Lines:
                    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                    break;
            }

            VkViewport viewport;
            viewport.x = 0;
            viewport.y = 0;
            viewport.width = static_cast<float>(swapchain_extent.width);
            viewport.height = static_cast<float>(swapchain_extent.height);
            viewport.minDepth = 0.0F;
            viewport.maxDepth = 1.0F;

            VkRect2D scissor;
            scissor.offset = {0, 0};
            scissor.extent = swapchain_extent;

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
            rasterizer_create_info.depthBiasEnable = VK_TRUE;
            rasterizer_create_info.depthClampEnable = VK_FALSE;
            rasterizer_create_info.depthBiasConstantFactor = data.depth_bias;
            rasterizer_create_info.depthBiasClamp = 0.0F;
            rasterizer_create_info.depthBiasSlopeFactor = data.slope_scaled_depth_bias;

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
            depth_stencil_create_info.depthTestEnable = static_cast<VkBool32>(
                std::find(data.states.begin(), data.states.end(), state_enum::DisableDepthTest) == data.states.end());
            depth_stencil_create_info.depthWriteEnable = static_cast<VkBool32>(
                std::find(data.states.begin(), data.states.end(), state_enum::DisableDepthWrite) == data.states.end());
            depth_stencil_create_info.depthCompareOp = vulkan::type_converters::to_compare_op(data.depth_func);
            depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
            depth_stencil_create_info.stencilTestEnable = static_cast<VkBool32>(
                std::find(data.states.begin(), data.states.end(), state_enum::EnableStencilTest) != data.states.end());
            if(data.front_face) {
                depth_stencil_create_info.front.failOp = vulkan::type_converters::to_stencil_op(data.front_face->fail_op);
                depth_stencil_create_info.front.passOp = vulkan::type_converters::to_stencil_op(data.front_face->pass_op);
                depth_stencil_create_info.front.depthFailOp = vulkan::type_converters::to_stencil_op(data.front_face->depth_fail_op);
                depth_stencil_create_info.front.compareOp = vulkan::type_converters::to_compare_op(data.front_face->compare_op);
                depth_stencil_create_info.front.compareMask = data.front_face->compare_mask;
                depth_stencil_create_info.front.writeMask = data.front_face->write_mask;
            }
            if(data.back_face) {
                depth_stencil_create_info.back.failOp = vulkan::type_converters::to_stencil_op(data.back_face->fail_op);
                depth_stencil_create_info.back.passOp = vulkan::type_converters::to_stencil_op(data.back_face->pass_op);
                depth_stencil_create_info.back.depthFailOp = vulkan::type_converters::to_stencil_op(data.back_face->depth_fail_op);
                depth_stencil_create_info.back.compareOp = vulkan::type_converters::to_compare_op(data.back_face->compare_op);
                depth_stencil_create_info.back.compareMask = data.back_face->compare_mask;
                depth_stencil_create_info.back.writeMask = data.back_face->write_mask;
            }

            VkPipelineColorBlendAttachmentState color_blend_attachment;
            color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                                    VK_COLOR_COMPONENT_A_BIT;
            color_blend_attachment.blendEnable = VK_TRUE;
            color_blend_attachment.srcColorBlendFactor = vulkan::type_converters::to_blend_factor(data.source_blend_factor);
            color_blend_attachment.dstColorBlendFactor = vulkan::type_converters::to_blend_factor(data.destination_blend_factor);
            color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
            color_blend_attachment.srcAlphaBlendFactor = vulkan::type_converters::to_blend_factor(data.alpha_src);
            color_blend_attachment.dstAlphaBlendFactor = vulkan::type_converters::to_blend_factor(data.alpha_dst);
            color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo color_blend_create_info;
            color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend_create_info.pNext = nullptr;
            color_blend_create_info.flags = 0;
            color_blend_create_info.logicOpEnable = VK_FALSE;
            color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
            color_blend_create_info.attachmentCount = 1;
            color_blend_create_info.pAttachments = &color_blend_attachment;
            color_blend_create_info.blendConstants[0] = 0.0F;
            color_blend_create_info.blendConstants[1] = 0.0F;
            color_blend_create_info.blendConstants[2] = 0.0F;
            color_blend_create_info.blendConstants[3] = 0.0F;

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
            pipeline_create_info.pDynamicState = nullptr;
            pipeline_create_info.layout = layout;
            pipeline_create_info.renderPass = render_passes.at(data.pass).pass;
            pipeline_create_info.subpass = 0;
            pipeline_create_info.basePipelineIndex = -1;

            NOVA_THROW_IF_VK_ERROR(vkCreateGraphicsPipelines(device,
                                                             VK_NULL_HANDLE,
                                                             1,
                                                             &pipeline_create_info,
                                                             nullptr,
                                                             &nova_pipeline.pipeline),
                                   render_engine_initialization_exception);

            pipelines_by_renderpass[data.pass].push_back(nova_pipeline);

            if(settings.debug.enabled) {
                VkDebugUtilsObjectNameInfoEXT object_name = {};
                object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                object_name.objectType = VK_OBJECT_TYPE_IMAGE;
                object_name.objectHandle = reinterpret_cast<uint64_t>(nova_pipeline.pipeline);
                object_name.pObjectName = data.name.c_str();
                NOVA_THROW_IF_VK_ERROR(vkSetDebugUtilsObjectNameEXT(device, &object_name), render_engine_initialization_exception);
                NOVA_LOG(INFO) << "Set pipeline " << nova_pipeline.pipeline << " to have name " << data.name;
            }
        }
    }

    VkShaderModule vulkan_render_engine::create_shader_module(const std::vector<uint32_t>& spirv) const {
        VkShaderModuleCreateInfo shader_module_create_info;
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.pNext = nullptr;
        shader_module_create_info.flags = 0;
        shader_module_create_info.pCode = spirv.data();
        shader_module_create_info.codeSize = spirv.size() * 4;

        VkShaderModule module;
        NOVA_THROW_IF_VK_ERROR(vkCreateShaderModule(device, &shader_module_create_info, nullptr, &module),
                               render_engine_initialization_exception);

        return module;
    }

    void vulkan_render_engine::get_shader_module_descriptors(const std::vector<uint32_t>& spirv,
                                                             const VkShaderStageFlags shader_stage,
                                                             std::unordered_map<std::string, vk_resource_binding>& bindings) {
        const spirv_cross::CompilerGLSL shader_compiler(spirv);
        const spirv_cross::ShaderResources resources = shader_compiler.get_shader_resources();

        for(const spirv_cross::Resource& resource : resources.sampled_images) {
            NOVA_LOG(TRACE) << "Found a texture resource named " << resource.name;
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        }

        for(const spirv_cross::Resource& resource : resources.uniform_buffers) {
            NOVA_LOG(TRACE) << "Found a UBO resource named " << resource.name;
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        }

        for(const spirv_cross::Resource& resource : resources.storage_buffers) {
            NOVA_LOG(TRACE) << "Found a SSBO resource named " << resource.name;
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        }
    }

    void vulkan_render_engine::add_resource_to_bindings(std::unordered_map<std::string, vk_resource_binding>& bindings,
                                                        const VkShaderStageFlags shader_stage,
                                                        const spirv_cross::CompilerGLSL& shader_compiler,
                                                        const spirv_cross::Resource& resource,
                                                        const VkDescriptorType type) {
        const uint32_t set = shader_compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        const uint32_t binding = shader_compiler.get_decoration(resource.id, spv::DecorationBinding);

        vk_resource_binding new_binding = {};
        new_binding.set = set;
        new_binding.binding = binding;
        new_binding.descriptorType = type;
        new_binding.descriptorCount = 1;
        new_binding.stageFlags = shader_stage;

        if(bindings.find(resource.name) == bindings.end()) {
            // Totally new binding!
            bindings[resource.name] = new_binding;
        } else {
            // Existing binding. Is it the same as our binding?
            vk_resource_binding& existing_binding = bindings.at(resource.name);
            if(existing_binding != new_binding) {
                // They have two different bindings with the same name. Not allowed
                NOVA_LOG(ERROR) << "You have two different uniforms named " << resource.name
                                << " in different shader stages. This is not allowed. Use unique names";

            } else {
                // Same binding, probably at different stages - let's fix that
                existing_binding.stageFlags |= shader_stage;
            }
        }
    }

    std::vector<VkDescriptorSetLayout> vulkan_render_engine::create_descriptor_set_layouts(
        const std::unordered_map<std::string, vk_resource_binding>& all_bindings) const {

        /*
         * A few tasks to accomplish:
         * - Take the unordered map of descriptor sets (all_bindings) and convert it into
         *      VkDescriptorSetLayoutCreateInfo structs, ordering everything along the way
         * -
         */

        std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindings_by_set;
        bindings_by_set.resize(all_bindings.size());

        for(const auto& named_binding : all_bindings) {
            const vk_resource_binding& binding = named_binding.second;
            if(binding.set >= bindings_by_set.size()) {
                NOVA_LOG(ERROR) << "You've skipped one or more descriptor sets! Don't do that, Nova can't handle it";
                continue;
            }

            bindings_by_set[binding.set].push_back(binding);
        }

        std::vector<VkDescriptorSetLayoutCreateInfo> dsl_create_infos = {};
        dsl_create_infos.reserve(bindings_by_set.size());
        for(const auto& bindings : bindings_by_set) {
            VkDescriptorSetLayoutCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            create_info.bindingCount = static_cast<uint32_t>(bindings.size());
            create_info.pBindings = bindings.data();

            dsl_create_infos.push_back(create_info);
        }

        std::vector<VkDescriptorSetLayout> layouts;
        layouts.resize(dsl_create_infos.size());
        for(size_t i = 0; i < dsl_create_infos.size(); i++) {
            vkCreateDescriptorSetLayout(device, &dsl_create_infos[i], nullptr, &layouts[i]);
        }

        return layouts;
    }

    void vulkan_render_engine::create_material_descriptor_sets() {
        for(const auto& [renderpass_name, pipelines] : pipelines_by_renderpass) {
            (void) renderpass_name;
            for(const auto& pipeline : pipelines) {
                std::vector<material_pass>& material_passes = material_passes_by_pipeline.at(pipeline.data.name);
                for(material_pass& mat_pass : material_passes) {
                    if(pipeline.layouts.empty()) {
                        // If there's no layouts, we're done
                        NOVA_LOG(TRACE) << "No layouts for pipeline " << pipeline.data.name << ", which material pass " << mat_pass.name
                                        << " of material " << mat_pass.material_name << " uses";
                        continue;
                    }

                    NOVA_LOG(TRACE) << "Creating descriptor sets for pipeline " << pipeline.data.name;

                    auto layouts = std::vector<VkDescriptorSetLayout>{};
                    layouts.reserve(pipeline.layouts.size());

                    // CLion might tell you to simplify this into a foreach loop... DO NOT! The layouts need to be added in set
                    // order, not map order which is what you'll get if you use a foreach - AND IT'S WRONG
                    for(size_t i = 0; i < pipeline.layouts.size(); i++) {
                        layouts.push_back(pipeline.layouts[static_cast<uint32_t>(i)]);
                    }

                    VkDescriptorSetAllocateInfo alloc_info = {};
                    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    alloc_info.descriptorPool = get_descriptor_pool_for_current_thread();
                    alloc_info.descriptorSetCount = static_cast<uint32_t>(layouts.size());
                    alloc_info.pSetLayouts = layouts.data();

                    mat_pass.descriptor_sets.resize(layouts.size());
                    NOVA_THROW_IF_VK_ERROR(vkAllocateDescriptorSets(device, &alloc_info, mat_pass.descriptor_sets.data()),
                                           shaderpack_loading_error);

                    std::stringstream ss;
                    for(const VkDescriptorSet& set : mat_pass.descriptor_sets) {
                        ss << set << ", ";
                    }
                    NOVA_LOG(TRACE) << "Material pass " << mat_pass.name << " in material " << mat_pass.material_name << " has descriptors [" << ss.str() << "]";

                    update_material_descriptor_sets(mat_pass, pipeline.bindings);
                }
            }
        }
    }

    void vulkan_render_engine::update_material_descriptor_sets(
        const material_pass& mat, const std::unordered_map<std::string, vk_resource_binding>& name_to_descriptor) {
        // for each resource:
        //  - Get its set and binding from the pipeline
        //  - Update its descriptor set
        NOVA_LOG(TRACE) << "Updating descriptors for material " << mat.material_name;

        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(mat.bindings.size());

        // We create VkDescriptorImageInfo objects in a different scope, so were they to live there forever they'd get destructed before
        // we can use them Instead we have them in a std::vector so they get deallocated _after_ being used
        std::vector<VkDescriptorImageInfo> image_infos;
        image_infos.reserve(mat.bindings.size());

        std::vector<VkDescriptorBufferInfo> buffer_infos;
        buffer_infos.reserve(mat.bindings.size());

        for(const auto& [renderpass_name, pipelines] : pipelines_by_renderpass) {
            (void) renderpass_name;
            for(const vk_pipeline& pipeline : pipelines) {
                if(pipeline.data.name == mat.pipeline) {
                    for(const auto& [descriptor_name, resource_name] : mat.bindings) {
                        if(pipeline.bindings.find(descriptor_name) == pipeline.bindings.end()) {
                            NOVA_LOG(DEBUG) << "Material pass " << mat.name << " in material " << mat.material_name << " wants to bind "
                                            << resource_name << " to descriptor set " << descriptor_name
                                            << ", but it doesn't exist in pipeline " << pipeline.data.name
                                            << ", which this material pass uses";
                        }
                    }

                    break;
                }
            }
        }

        for(const auto& [descriptor_name, resource_name] : mat.bindings) {
            if(name_to_descriptor.find(descriptor_name) == name_to_descriptor.end()) {
                NOVA_LOG(ERROR) << "Descriptor " << descriptor_name << " is not known to Nova, probably because you have it in your material but not in your pipeline";
                continue;
            }
            const auto& descriptor_info = name_to_descriptor.at(descriptor_name);
            const auto descriptor_set = mat.descriptor_sets.at(descriptor_info.set);
            bool is_known = true;

            VkWriteDescriptorSet write = {};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptor_set;
            write.dstBinding = descriptor_info.binding;
            write.descriptorCount = 1;
            write.dstArrayElement = 0;

            if(textures.find(resource_name) != textures.end()) {
                NOVA_LOG(TRACE) << "Binding  texture " << resource_name << " to descriptor (" << write.dstSet << "." << write.dstBinding << ")";
                const vk_texture& texture = textures.at(resource_name);
                write_texture_to_descriptor(texture, write, image_infos);

            } else if(buffers.find(resource_name) != buffers.end()) {
                NOVA_LOG(TRACE) << "Binding dynamic buffer " << resource_name << " to descriptor (" << write.dstSet << "." << write.dstBinding << ")";
                const vk_buffer& buffer = buffers.at(resource_name);
                write_buffer_to_descriptor(buffer.buffer, write, buffer_infos, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

            } else if(resource_name == "NovaModelMatrixBuffer") {
                NOVA_LOG(TRACE) << "Binding buffer NovaModelMatrixBuffer to descriptor (" << write.dstSet << "." << write.dstBinding << ")";
                write_buffer_to_descriptor(model_matrix_buffer->get_vk_buffer(), write, buffer_infos, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

            } else if(resource_name == "NovaPerFrameUBO") {
                NOVA_LOG(TRACE) << "Binding buffer NovaPerFrameUBO to descriptor (" << write.dstSet << "." << write.dstBinding << ")";
                write_buffer_to_descriptor(per_frame_data_buffer->get_vk_buffer(), write, buffer_infos, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

            } else {
                is_known = false;
                NOVA_LOG(WARN) << "Resource " << resource_name
                               << " is not known to Nova. I hope you aren't using it cause it doesn't exist";
            }

            if(is_known) {
                writes.push_back(write);
            }
        }

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }

    void vulkan_render_engine::write_texture_to_descriptor(const vk_texture& texture,
                                                           VkWriteDescriptorSet& write,
                                                           std::vector<VkDescriptorImageInfo>& image_infos) const {
        VkDescriptorImageInfo image_info = {};
        image_info.imageView = texture.image_view;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.sampler = point_sampler;

        image_infos.push_back(image_info);

        write.pImageInfo = &image_infos.at(image_infos.size() - 1);
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }

    void vulkan_render_engine::write_buffer_to_descriptor(const VkBuffer& buffer,
                                                          VkWriteDescriptorSet& write,
                                                          std::vector<VkDescriptorBufferInfo>& buffer_infos,
                                                          const VkDescriptorType type) {
        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer;
        buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE;

        buffer_infos.push_back(buffer_info);

        write.pBufferInfo = &buffer_infos[buffer_infos.size() - 1];
        write.descriptorType = type;
    }

    void vulkan_render_engine::generate_barriers_for_dynamic_resources() {
        for(auto& [name, pass] : render_passes) {
            (void) name;
            create_barriers_for_renderpass(pass);
        }
    }

    void vulkan_render_engine::create_barriers_for_renderpass(vk_render_pass& pass) {
        /*
         * For each renderpass:
         * - Walk backwards through previous renderpasses
         * - If we find one that writes to a resource we read from before we find one that reads from a resource we read from, add a
         * barrier
         * - If we find one that reads from a resource we write to before we find one that writes to a resource we write to, add a
         * barrier
         */

        std::unordered_map<std::string, barrier_necessity> read_texture_barrier_necessity;
        read_texture_barrier_necessity.reserve(pass.data.texture_inputs.size());
        for(const std::string& tex_name : pass.data.texture_inputs) {
            read_texture_barrier_necessity[tex_name] = barrier_necessity::maybe;
        }

        std::unordered_map<std::string, barrier_necessity> write_texture_barrier_necessity;
        write_texture_barrier_necessity.reserve(pass.data.texture_outputs.size());
        for(const texture_attachment& attach : pass.data.texture_outputs) {
            write_texture_barrier_necessity[attach.name] = barrier_necessity::maybe;
        }

        // Find where we are in the list of passes
        uint64_t idx = 0;
        for(; idx < render_passes_by_order.size(); idx++) {
            if(render_passes_by_order.at(idx) == pass.data.name) {
                break;
            }
        }

        // Walk backwards from where we are, checking if any of the textures the previous pass writes to are textures we need to barrier
        while(true) {
            const std::string& prev_pass_name = render_passes_by_order.at(idx);
            const vk_render_pass& previous_pass = render_passes.at(prev_pass_name);
            // If the previous pass reads from a texture that we read from, it (or a even earlier pass) will have the barrier
            const std::vector<std::string> read_textures = previous_pass.data.texture_inputs;

            for(const std::string& prev_read_texture : read_textures) {
                // If the previous pass reads from a texture that we read from, we don't need to barrier it
                if(read_texture_barrier_necessity.find(prev_read_texture) != read_texture_barrier_necessity.end()) {
                    if(read_texture_barrier_necessity.at(prev_read_texture) == barrier_necessity::maybe) {
                        NOVA_LOG(TRACE) << "Do not need a barrier for read texture " << prev_read_texture << " before renderpass "
                                        << pass.data.name << " because pass " << previous_pass.data.name << " reads from it as well";
                        read_texture_barrier_necessity[prev_read_texture] = barrier_necessity::no;
                    }
                }

                // If the previous pass reads from a texture that we write to, we need a barrier
                if(write_texture_barrier_necessity.find(prev_read_texture) != write_texture_barrier_necessity.end()) {
                    if(write_texture_barrier_necessity.at(prev_read_texture) == barrier_necessity::maybe) {
                        NOVA_LOG(TRACE) << "Need a barrier for write texture " << prev_read_texture << " before renderpass "
                                        << pass.data.name << " because pass " << previous_pass.data.name << " reads from it";
                        write_texture_barrier_necessity[prev_read_texture] = barrier_necessity::no;
                    }
                }
            }

            for(const texture_attachment& prev_write_tex : previous_pass.data.texture_outputs) {
                // If the previous pass write to a texture that we read from, we need a barrier
                if(read_texture_barrier_necessity.find(prev_write_tex.name) != read_texture_barrier_necessity.end()) {
                    if(read_texture_barrier_necessity.at(prev_write_tex.name) == barrier_necessity::maybe) {
                        NOVA_LOG(TRACE) << "Need a barrier for read texture " << prev_write_tex.name << " before renderpass "
                                        << pass.data.name << " because pass " << previous_pass.data.name << " writes to is";
                        read_texture_barrier_necessity[prev_write_tex.name] = barrier_necessity::yes;
                    }
                }

                // If the previous pass write to a texture that we write to, we don't need to barrier it
                if(write_texture_barrier_necessity.find(prev_write_tex.name) != write_texture_barrier_necessity.end()) {
                    if(write_texture_barrier_necessity.at(prev_write_tex.name) == barrier_necessity::maybe) {
                        NOVA_LOG(TRACE) << "Do not need a barrier for write texture " << prev_write_tex.name << " before renderpass "
                                        << pass.data.name << " because pass " << previous_pass.data.name << " writes to it as well";
                        write_texture_barrier_necessity[prev_write_tex.name] = barrier_necessity::yes;
                    }
                }
            }

            if(idx == 0) {
                break;
            }
            idx--;
        }

        for(const auto& [tex_name, _] : read_texture_barrier_necessity) {
            (void) _;
            const vk_texture& tex = textures.at(tex_name);

            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = graphics_family_index;
            barrier.dstQueueFamilyIndex = graphics_family_index;
            barrier.image = tex.image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            pass.read_texture_barriers.push_back(barrier);
        }

        for(const auto& [tex_name, _] : write_texture_barrier_necessity) {
            (void) _;
            if(tex_name == "Backbuffer") {
                continue;
            }

            VkImage image = textures.at(tex_name).image;

            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.srcQueueFamilyIndex = graphics_family_index;
            barrier.dstQueueFamilyIndex = graphics_family_index;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            pass.write_texture_barriers.push_back(barrier);
        }
    }
} // namespace nova::renderer
