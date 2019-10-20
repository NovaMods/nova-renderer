#include "vulkan_render_engine.hpp"
#include "../../util/logger.hpp"
#include "vk_structs.hpp"
#include "vulkan_command_list.hpp"
#include "vulkan_utils.hpp"

#include <set>
#include "nova_renderer/allocation_structs.hpp"

#ifdef NOVA_LINUX
#define VK_USE_PLATFORM_XLIB_KHR // Use X11 for window creating on Linux... TODO: Wayland?
#define NOVA_VK_XLIB
#include "../../windowing/x11_window.hpp"
#include <vulkan/vulkan_xlib.h>
#include "../../util/linux_utils.hpp"

#elif defined(NOVA_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#define NOVA_USE_WIN32
#include "../../util/windows.hpp"
#include <vulkan/vulkan_win32.h>
#include "../../windowing/win32_window.hpp"
#endif

namespace nova::renderer::rhi {
    VulkanRenderEngine::VulkanRenderEngine(NovaSettingsAccessManager& settings)
        : RenderEngine(&mallocator, settings) { // NOLINT(cppcoreguidelines-pro-type-member-init)
        create_instance();

        if(settings.settings.debug.enabled) {
            enable_debug_output();
        }

        open_window_and_create_surface(settings.settings.window);

        create_device_and_queues();

        if(settings.settings.debug.enabled) {
            // Late init, can only be used when the device has already been created
            vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
                    vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));
        }

        create_swapchain();

        create_per_thread_command_pools();
    }

    void VulkanRenderEngine::set_num_renderpasses(uint32_t /* num_renderpasses */) {
        // Pretty sure Vulkan doesn't need to do anything here
    }

    ntl::Result<DeviceMemory*> VulkanRenderEngine::allocate_device_memory(const uint64_t size,
                                                                          const MemoryUsage usage,
                                                                          const ObjectType /* allowed_objects */) {
        auto* memory = new_object<VulkanDeviceMemory>();

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = size;
        alloc_info.memoryTypeIndex = VK_MAX_MEMORY_TYPES;

        // Find the memory type that we want
        switch(usage) {
            case MemoryUsage::DeviceOnly:
                // Find a memory type that only has the device local bit set
                // If none have only the device local bit set, find one with the device local but and maybe other things
                alloc_info.memoryTypeIndex = find_memory_type_with_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, MemorySearchMode::Exact);
                if(alloc_info.memoryTypeIndex == VK_MAX_MEMORY_TYPES) {
                    alloc_info.memoryTypeIndex = find_memory_type_with_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                }
                break;

            case MemoryUsage::LowFrequencyUpload:
                // Find a memory type that's visible to both the device and the host. Memory that's both device local and host visible would
                // be amazing, otherwise HOST_CACHED will work I guess
                alloc_info.memoryTypeIndex = find_memory_type_with_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
                if(alloc_info.memoryTypeIndex == VK_MAX_MEMORY_TYPES) {
                    alloc_info.memoryTypeIndex = find_memory_type_with_flags(VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
                }
                break;

            case MemoryUsage::StagingBuffer:
                alloc_info.memoryTypeIndex = find_memory_type_with_flags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                         VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
                break;
        }

        vkAllocateMemory(device, &alloc_info, nullptr, &memory->memory);

        if(usage == MemoryUsage::LowFrequencyUpload || usage == MemoryUsage::StagingBuffer) {
            void* mapped_memory;
            vkMapMemory(device, memory->memory, 0, VK_WHOLE_SIZE, 0, &mapped_memory);
            heap_mappings.emplace(memory->memory, mapped_memory);
        }

        return ntl::Result<DeviceMemory*>(memory);
    }

    ntl::Result<Renderpass*> VulkanRenderEngine::create_renderpass(const shaderpack::RenderPassCreateInfo& data) {
        auto* vk_swapchain = static_cast<VulkanSwapchain*>(swapchain);
        VkExtent2D swapchain_extent = {swapchain_size.x, swapchain_size.y};

        auto* renderpass = new_object<VulkanRenderpass>();

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

        std::vector<VkAttachmentReference> attachment_references;
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkImageView> framebuffer_attachments;
        uint32_t framebuffer_width = 0;
        uint32_t framebuffer_height = 0;

        bool writes_to_backbuffer = false;
        // Collect framebuffer size information from color output attachments
        for(const shaderpack::TextureAttachmentInfo& attachment : data.texture_outputs) {
            if(attachment.name == "Backbuffer") {
                // Handle backbuffer
                // Backbuffer framebuffers are handled by themselves in their own special snowflake way so we just need to skip
                // everything
                writes_to_backbuffer = true;

                VkAttachmentDescription desc = {};
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

                VkAttachmentReference ref = {};

                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                ref.attachment = static_cast<uint32_t>(attachments.size()) - 1;

                attachment_references.push_back(ref);

                framebuffer_width = swapchain_extent.width;
                framebuffer_height = swapchain_extent.height;

                renderpass->writes_to_backbuffer = true;

                break;
            }

            VkAttachmentDescription desc = {};
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

            VkAttachmentReference ref = {};

            ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ref.attachment = static_cast<uint32_t>(attachments.size()) - 1;

            attachment_references.push_back(ref);
        }

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
            return ntl::Result<Renderpass*>(MAKE_ERROR(
                "Framebuffer width for pass {:s} is 0. This is illegal! Make sure that there is at least one attachment for this render pass, and ensure that all attachments used by this pass have a non-zero width",
                data.name));
        }

        if(framebuffer_height == 0) {
            return ntl::Result<Renderpass*>(MAKE_ERROR(
                "Framebuffer height for pass {:s} is 0. This is illegal! Make sure that there is at least one attachment for this render pass, and ensure that all attachments used by this pass have a non-zero height",
                data.name));
        }

        if(framebuffer_attachments.size() > gpu.props.limits.maxColorAttachments) {
            return ntl::Result<Renderpass*>(MAKE_ERROR(
                "Framebuffer for pass {:s} has {:d} color attachments, but your GPU only supports {:d}. Please reduce the number of attachments that this pass uses, possibly by changing some of your input attachments to bound textures",
                data.name,
                data.texture_outputs.size(),
                gpu.props.limits.maxColorAttachments));
        }

        subpass_description.colorAttachmentCount = static_cast<uint32_t>(attachment_references.size());
        subpass_description.pColorAttachments = attachment_references.data();

        render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_create_info.pAttachments = attachments.data();

        NOVA_CHECK_RESULT(vkCreateRenderPass(device, &render_pass_create_info, nullptr, &renderpass->pass));

        if(writes_to_backbuffer) {
            if(data.texture_outputs.size() > 1) {
                NOVA_LOG(ERROR)
                    << "Pass " << data.name.c_str()
                    << " writes to the backbuffer, and other textures. Passes that write to the backbuffer are not allowed to write to any other textures";
            }
        }

        renderpass->render_area = {{0, 0}, {framebuffer_width, framebuffer_height}};

        if(settings.settings.debug.enabled) {
            VkDebugUtilsObjectNameInfoEXT object_name = {};
            object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            object_name.objectType = VK_OBJECT_TYPE_IMAGE;
            object_name.objectHandle = reinterpret_cast<uint64_t>(renderpass->pass);
            object_name.pObjectName = data.name.c_str();
            NOVA_CHECK_RESULT(vkSetDebugUtilsObjectNameEXT(device, &object_name));
        }

        return ntl::Result(static_cast<Renderpass*>(renderpass));
    }

    Framebuffer* VulkanRenderEngine::create_framebuffer(const Renderpass* renderpass,
                                                        const std::vector<Image*>& attachments,
                                                        const glm::uvec2& framebuffer_size) {
        const VulkanRenderpass* vk_renderpass = static_cast<const VulkanRenderpass*>(renderpass);

        std::vector<VkImageView> attachment_views;
        attachment_views.reserve(attachments.size());
        for(const Image* attachment : attachments) {
            const VulkanImage* vk_image = static_cast<const VulkanImage*>(attachment);
            attachment_views.push_back(vk_image->image_view);
        }

        VkFramebufferCreateInfo framebuffer_create_info = {};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = vk_renderpass->pass;
        framebuffer_create_info.attachmentCount = static_cast<uint32_t>(attachment_views.size());
        framebuffer_create_info.pAttachments = attachment_views.data();
        framebuffer_create_info.width = framebuffer_size.x;
        framebuffer_create_info.height = framebuffer_size.y;
        framebuffer_create_info.layers = 1;

        VulkanFramebuffer* framebuffer = new_object<VulkanFramebuffer>();
        framebuffer->size = framebuffer_size;
        NOVA_CHECK_RESULT(vkCreateFramebuffer(device, &framebuffer_create_info, nullptr, &framebuffer->framebuffer));

        return framebuffer;
    }

    ntl::Result<PipelineInterface*> VulkanRenderEngine::create_pipeline_interface(
        const std::unordered_map<std::string, ResourceBindingDescription>& bindings,
        const std::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
        const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) {

        VulkanSwapchain* vk_swapchain = static_cast<VulkanSwapchain*>(swapchain);

        VulkanPipelineInterface* pipeline_interface = new_object<VulkanPipelineInterface>();
        pipeline_interface->bindings = bindings;

        pipeline_interface->layouts_by_set = create_descriptor_set_layouts(bindings);

        VkPipelineLayoutCreateInfo pipeline_layout_create_info;
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.pNext = nullptr;
        pipeline_layout_create_info.flags = 0;
        pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(pipeline_interface->layouts_by_set.size());
        pipeline_layout_create_info.pSetLayouts = pipeline_interface->layouts_by_set.data();
        pipeline_layout_create_info.pushConstantRangeCount = 0;
        pipeline_layout_create_info.pPushConstantRanges = nullptr;

        NOVA_CHECK_RESULT(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipeline_interface->pipeline_layout));

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
        std::vector<VkAttachmentDescription> attachment_descriptions;
        std::vector<VkImageView> framebuffer_attachments;
        uint32_t framebuffer_width = 0;
        uint32_t framebuffer_height = 0;

        bool writes_to_backbuffer = false;
        // Collect framebuffer size information from color output attachments
        for(const shaderpack::TextureAttachmentInfo& attachment : color_attachments) {
            if(attachment.name == "Backbuffer") {
                // Handle backbuffer
                // Backbuffer framebuffers are handled by themselves in their own special snowflake way so we just need to skip
                // everything
                writes_to_backbuffer = true;

                VkAttachmentDescription desc = {};
                desc.flags = 0;
                desc.format = vk_swapchain->get_swapchain_format();
                desc.samples = VK_SAMPLE_COUNT_1_BIT;
                desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                attachment_descriptions.push_back(desc);

                VkAttachmentReference ref = {};

                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                ref.attachment = static_cast<uint32_t>(attachment_descriptions.size()) - 1;

                attachment_references.push_back(ref);

                break;
            }

            VkAttachmentDescription desc = {};
            desc.flags = 0;
            desc.format = to_vk_format(attachment.pixel_format);
            desc.samples = VK_SAMPLE_COUNT_1_BIT;
            desc.loadOp = attachment.clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            attachment_descriptions.push_back(desc);

            VkAttachmentReference ref = {};

            ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ref.attachment = static_cast<uint32_t>(attachment_descriptions.size()) - 1;

            attachment_references.push_back(ref);
        }

        VkAttachmentReference depth_reference = {};
        // Collect framebuffer size information from the depth attachment
        if(depth_texture) {
            VkAttachmentDescription desc = {};
            desc.flags = 0;
            desc.format = to_vk_format(depth_texture->pixel_format);
            desc.samples = VK_SAMPLE_COUNT_1_BIT;
            desc.loadOp = depth_texture->clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            attachment_descriptions.push_back(desc);

            depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depth_reference.attachment = static_cast<uint32_t>(attachment_descriptions.size()) - 1;

            subpass_description.pDepthStencilAttachment = &depth_reference;
        }

        subpass_description.colorAttachmentCount = static_cast<uint32_t>(attachment_references.size());
        subpass_description.pColorAttachments = attachment_references.data();

        render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachment_descriptions.size());
        render_pass_create_info.pAttachments = attachment_descriptions.data();

        NOVA_CHECK_RESULT(vkCreateRenderPass(device, &render_pass_create_info, nullptr, &pipeline_interface->pass));

        return ntl::Result(static_cast<PipelineInterface*>(pipeline_interface));
    }

    DescriptorPool* VulkanRenderEngine::create_descriptor_pool(const uint32_t num_sampled_images,
                                                               const uint32_t num_samplers,
                                                               const uint32_t num_uniform_buffers) {
        std::vector<VkDescriptorPoolSize> pool_sizes;
        pool_sizes.emplace_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, num_sampled_images});
        pool_sizes.emplace_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER, num_samplers});
        pool_sizes.emplace_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, num_uniform_buffers});

        VkDescriptorPoolCreateInfo pool_create_info = {};
        pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_create_info.maxSets = num_sampled_images + num_samplers + num_uniform_buffers;
        pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        pool_create_info.pPoolSizes = pool_sizes.data();

        VulkanDescriptorPool* pool = new_object<VulkanDescriptorPool>();
        NOVA_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_create_info, nullptr, &pool->descriptor_pool));

        return pool;
    }

    std::vector<DescriptorSet*> VulkanRenderEngine::create_descriptor_sets(const PipelineInterface* pipeline_interface,
                                                                           DescriptorPool* pool) {
        const VulkanPipelineInterface* vk_pipeline_interface = static_cast<const VulkanPipelineInterface*>(pipeline_interface);
        const VulkanDescriptorPool* vk_pool = static_cast<const VulkanDescriptorPool*>(pool);

        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = vk_pool->descriptor_pool;
        alloc_info.descriptorSetCount = static_cast<uint32_t>(vk_pipeline_interface->layouts_by_set.size());
        alloc_info.pSetLayouts = vk_pipeline_interface->layouts_by_set.data();

        std::vector<VkDescriptorSet> sets;
        sets.resize(vk_pipeline_interface->layouts_by_set.size());
        vkAllocateDescriptorSets(device, &alloc_info, sets.data());

        std::vector<DescriptorSet*> final_sets;
        final_sets.reserve(sets.size());
        for(const VkDescriptorSet set : sets) {
            VulkanDescriptorSet* vk_set = new_object<VulkanDescriptorSet>();
            vk_set->descriptor_set = set;
            final_sets.push_back(vk_set);
        }

        return final_sets;
    }

    void VulkanRenderEngine::update_descriptor_sets(std::vector<DescriptorSetWrite>& writes) {
        std::vector<VkWriteDescriptorSet> vk_writes;
        vk_writes.reserve(writes.size());

        std::vector<VkDescriptorImageInfo> image_infos;
        image_infos.reserve(writes.size());

        for(const DescriptorSetWrite& write : writes) {
            VkWriteDescriptorSet vk_write = {};
            vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            vk_write.dstSet = static_cast<const VulkanDescriptorSet*>(write.set)->descriptor_set;
            vk_write.dstBinding = write.binding;
            vk_write.descriptorCount = 1;
            vk_write.dstArrayElement = 0;

            switch(write.type) {
                case DescriptorType::CombinedImageSampler: {
                    VkDescriptorImageInfo vk_image_info = {};
                    vk_image_info.imageView = image_view_for_image(write.image_info->image);
                    vk_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    vk_image_info.sampler = static_cast<VulkanSampler*>(write.image_info->sampler)->sampler;

                    image_infos.push_back(vk_image_info);

                    vk_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    vk_write.pImageInfo = &image_infos.at(image_infos.size() - 1);

                    vk_writes.push_back(vk_write);
                } break;

                case DescriptorType::UniformBuffer: {
                } break;

                case DescriptorType::StorageBuffer: {
                } break;

                default:;
            }
        }

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(vk_writes.size()), vk_writes.data(), 0, nullptr);
    }

    ntl::Result<Pipeline*> VulkanRenderEngine::create_pipeline(PipelineInterface* pipeline_interface,
                                                               const shaderpack::PipelineCreateInfo& data) {
        NOVA_LOG(TRACE) << "Creating a VkPipeline for pipeline " << data.name.c_str();

        const VulkanPipelineInterface* vk_interface = static_cast<const VulkanPipelineInterface*>(pipeline_interface);

        VulkanPipeline* vk_pipeline = new_object<VulkanPipeline>();

        std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
        std::unordered_map<VkShaderStageFlags, VkShaderModule> shader_modules;

        NOVA_LOG(TRACE) << "Compiling vertex module";
        shader_modules[VK_SHADER_STAGE_VERTEX_BIT] = create_shader_module(data.vertex_shader.source);

        if(data.geometry_shader) {
            NOVA_LOG(TRACE) << "Compiling geometry module";
            shader_modules[VK_SHADER_STAGE_GEOMETRY_BIT] = create_shader_module(data.geometry_shader->source);
        }

        if(data.tessellation_control_shader) {
            NOVA_LOG(TRACE) << "Compiling tessellation_control module";
            shader_modules[VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT] = create_shader_module(data.tessellation_control_shader->source);
        }

        if(data.tessellation_evaluation_shader) {
            NOVA_LOG(TRACE) << "Compiling tessellation_evaluation module";
            shader_modules[VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT] = create_shader_module(data.tessellation_evaluation_shader->source);
        }

        if(data.fragment_shader) {
            NOVA_LOG(TRACE) << "Compiling fragment module";
            shader_modules[VK_SHADER_STAGE_FRAGMENT_BIT] = create_shader_module(data.fragment_shader->source);
        }

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

        const std::vector<VkVertexInputBindingDescription>& vertex_binding_descriptions = get_vertex_input_binding_descriptions();
        const std::vector<VkVertexInputAttributeDescription>& vertex_attribute_descriptions = get_vertex_input_attribute_descriptions();

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
            case shaderpack::PrimitiveTopologyEnum::Triangles:
                input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                break;
            case shaderpack::PrimitiveTopologyEnum::Lines:
                input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                break;
        }

        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<float>(swapchain_size.x);
        viewport.height = static_cast<float>(swapchain_size.y);
        viewport.minDepth = 0.0F;
        viewport.maxDepth = 1.0F;

        VkRect2D scissor;
        scissor.offset = {0, 0};
        scissor.extent = {swapchain_size.x, swapchain_size.y};

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
            std::find(data.states.begin(), data.states.end(), shaderpack::StateEnum::DisableDepthTest) == data.states.end());
        depth_stencil_create_info.depthWriteEnable = static_cast<VkBool32>(
            std::find(data.states.begin(), data.states.end(), shaderpack::StateEnum::DisableDepthWrite) == data.states.end());
        depth_stencil_create_info.depthCompareOp = to_compare_op(data.depth_func);
        depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
        depth_stencil_create_info.stencilTestEnable = static_cast<VkBool32>(
            std::find(data.states.begin(), data.states.end(), shaderpack::StateEnum::EnableStencilTest) != data.states.end());
        if(data.front_face) {
            depth_stencil_create_info.front.failOp = to_stencil_op(data.front_face->fail_op);
            depth_stencil_create_info.front.passOp = to_stencil_op(data.front_face->pass_op);
            depth_stencil_create_info.front.depthFailOp = to_stencil_op(data.front_face->depth_fail_op);
            depth_stencil_create_info.front.compareOp = to_compare_op(data.front_face->compare_op);
            depth_stencil_create_info.front.compareMask = data.front_face->compare_mask;
            depth_stencil_create_info.front.writeMask = data.front_face->write_mask;
        }
        if(data.back_face) {
            depth_stencil_create_info.back.failOp = to_stencil_op(data.back_face->fail_op);
            depth_stencil_create_info.back.passOp = to_stencil_op(data.back_face->pass_op);
            depth_stencil_create_info.back.depthFailOp = to_stencil_op(data.back_face->depth_fail_op);
            depth_stencil_create_info.back.compareOp = to_compare_op(data.back_face->compare_op);
            depth_stencil_create_info.back.compareMask = data.back_face->compare_mask;
            depth_stencil_create_info.back.writeMask = data.back_face->write_mask;
        }

        VkPipelineColorBlendAttachmentState color_blend_attachment;
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                                VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_TRUE;
        color_blend_attachment.srcColorBlendFactor = to_blend_factor(data.source_blend_factor);
        color_blend_attachment.dstColorBlendFactor = to_blend_factor(data.destination_blend_factor);
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment.srcAlphaBlendFactor = to_blend_factor(data.alpha_src);
        color_blend_attachment.dstAlphaBlendFactor = to_blend_factor(data.alpha_dst);
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
        pipeline_create_info.layout = vk_interface->pipeline_layout;

        pipeline_create_info.renderPass = vk_interface->pass;
        pipeline_create_info.subpass = 0;
        pipeline_create_info.basePipelineIndex = -1;

        VkResult result = vkCreateGraphicsPipelines(device, nullptr, 1, &pipeline_create_info, nullptr, &vk_pipeline->pipeline);
        if(result != VK_SUCCESS) {
            return ntl::Result<Pipeline*>(MAKE_ERROR("Could not compile pipeline {:s}", data.name.c_str()));
        }

        if(settings.settings.debug.enabled) {
            VkDebugUtilsObjectNameInfoEXT object_name = {};
            object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            object_name.objectType = VK_OBJECT_TYPE_IMAGE;
            object_name.objectHandle = reinterpret_cast<uint64_t>(vk_pipeline->pipeline);
            object_name.pObjectName = data.name.c_str();
            NOVA_CHECK_RESULT(vkSetDebugUtilsObjectNameEXT(device, &object_name));
            NOVA_LOG(INFO) << "Set pipeline " << vk_pipeline->pipeline << " to have name " << data.name.c_str();
        }

        return ntl::Result(static_cast<Pipeline*>(vk_pipeline));
    }

    Buffer* VulkanRenderEngine::create_buffer(const BufferCreateInfo& info) {
        VulkanBuffer* buffer = new_object<VulkanBuffer>();

        VkBufferCreateInfo vk_create_info = {};
        vk_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vk_create_info.size = info.size;
        vk_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        switch(info.buffer_usage) {
            case BufferUsage::UniformBuffer: {
                if(info.size < gpu.props.limits.maxUniformBufferRange) {
                    vk_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

                } else {
                    vk_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
                }
            } break;

            case BufferUsage::IndexBuffer: {
                vk_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            } break;

            case BufferUsage::VertexBuffer: {
                vk_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            } break;

            case BufferUsage::StagingBuffer: {
                // TODO
                NOVA_LOG(FATAL) << "Unimplemented: tried to create staging buffer";
            } break;
        }

        VulkanDeviceMemory* vulkan_heap = static_cast<VulkanDeviceMemory*>(info.allocation.memory);

        vkCreateBuffer(device, &vk_create_info, nullptr, &buffer->buffer);
        vkBindBufferMemory(device, buffer->buffer, vulkan_heap->memory, info.allocation.allocation_info.offset.b_count());

        return buffer;
    }

    void VulkanRenderEngine::write_data_to_buffer(const void* data, const uint64_t num_bytes, const uint64_t offset, const Buffer* buffer) {
        const VulkanBuffer* vulkan_buffer = static_cast<const VulkanBuffer*>(buffer);

        const bvestl::polyalloc::AllocationInfo& allocation_info = vulkan_buffer->memory.allocation_info;
        const VulkanDeviceMemory* memory = static_cast<const VulkanDeviceMemory*>(vulkan_buffer->memory.memory);

        uint8_t* mapped_bytes = static_cast<uint8_t*>(heap_mappings.at(memory->memory)) + allocation_info.offset.b_count() + offset;
        memcpy(mapped_bytes, data, num_bytes);
    }

    Image* VulkanRenderEngine::create_texture(const shaderpack::TextureCreateInfo& info) {
        VulkanImage* texture = new_object<VulkanImage>();

        texture->is_dynamic = true;
        VkFormat format = to_vk_format(info.format.pixel_format);

        VkImageCreateInfo image_create_info = {};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = format;
        const glm::uvec2 texture_size = info.format.get_size_in_pixels(swapchain_size);
        image_create_info.extent.width = texture_size.x;
        image_create_info.extent.height = texture_size.y;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        if(format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT) {
            image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        } else {
            image_create_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        image_create_info.queueFamilyIndexCount = 1;
        image_create_info.pQueueFamilyIndices = &graphics_family_index;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        /*
        VmaAllocationCreateInfo alloc_create_info = {};
        alloc_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        alloc_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        alloc_create_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        alloc_create_info.preferredFlags = 0;
        alloc_create_info.memoryTypeBits = 0;
        alloc_create_info.pool = nullptr;
        alloc_create_info.pUserData = nullptr;

        // TODO: Actualy create the image
        // vmaCreateImage(vma_allocator, &image_create_info, &alloc_create_info, &texture->image, &texture->allocation,
        &texture->vma_info);*/

        VkImageViewCreateInfo image_view_create_info = {};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = texture->image;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = image_create_info.format;
        if(format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT) {
            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            texture->is_depth_tex = true;
        } else {
            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;

        vkCreateImageView(device, &image_view_create_info, nullptr, &texture->image_view);

        if(settings.settings.debug.enabled) {
            VkDebugUtilsObjectNameInfoEXT object_name = {};
            object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            object_name.objectType = VK_OBJECT_TYPE_IMAGE;
            object_name.objectHandle = reinterpret_cast<uint64_t>(texture->image);
            object_name.pObjectName = info.name.c_str();
            NOVA_CHECK_RESULT(vkSetDebugUtilsObjectNameEXT(device, &object_name));
            NOVA_LOG(INFO) << "Set image " << texture->image << " to have name " << info.name.c_str();
        }

        return texture;
    }

    Semaphore* VulkanRenderEngine::create_semaphore() { return nullptr; }

    std::vector<Semaphore*> VulkanRenderEngine::create_semaphores(uint32_t num_semaphores) { return std::vector<Semaphore*>(); }

    Fence* VulkanRenderEngine::create_fence(const bool signaled) {
        VulkanFence* fence = new_object<VulkanFence>();

        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        if(signaled) {
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }

        vkCreateFence(device, &fence_create_info, nullptr, &fence->fence);

        return fence;
    }

    std::vector<Fence*> VulkanRenderEngine::create_fences(const uint32_t num_fences, const bool signaled) {
        std::vector<Fence*> fences;
        fences.reserve(num_fences);

        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        if(signaled) {
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }

        for(uint32_t i = 0; i < num_fences; i++) {
            VulkanFence* fence = new_object<VulkanFence>();
            vkCreateFence(device, &fence_create_info, nullptr, &fence->fence);

            fences.push_back(fence);
        }

        return fences;
    }

    void VulkanRenderEngine::wait_for_fences(const std::vector<Fence*> fences) {
        vkWaitForFences(device,
                        static_cast<uint32_t>(fences.size()),
                        &(static_cast<VulkanFence*>(fences[0])->fence),
                        VK_TRUE,
                        std::numeric_limits<uint64_t>::max());
    }

    void VulkanRenderEngine::reset_fences(const std::vector<Fence*>& fences) {
        vkResetFences(device, static_cast<uint32_t>(fences.size()), reinterpret_cast<const VkFence*>(fences.data()));
    }

    void VulkanRenderEngine::destroy_renderpass(Renderpass* pass) {
        VulkanRenderpass* vk_renderpass = static_cast<VulkanRenderpass*>(pass);
        vkDestroyRenderPass(device, vk_renderpass->pass, nullptr);
    }

    void VulkanRenderEngine::destroy_framebuffer(Framebuffer* framebuffer) {
        const VulkanFramebuffer* vk_framebuffer = static_cast<const VulkanFramebuffer*>(framebuffer);
        vkDestroyFramebuffer(device, vk_framebuffer->framebuffer, nullptr);

        delete vk_framebuffer;
    }

    void VulkanRenderEngine::destroy_pipeline_interface(PipelineInterface* pipeline_interface) {
        VulkanPipelineInterface* vk_interface = static_cast<VulkanPipelineInterface*>(pipeline_interface);
        vkDestroyRenderPass(device, vk_interface->pass, nullptr);
        vkDestroyPipelineLayout(device, vk_interface->pipeline_layout, nullptr);
    }

    void VulkanRenderEngine::destroy_pipeline(Pipeline* pipeline) {
        VulkanPipeline* vk_pipeline = static_cast<VulkanPipeline*>(pipeline);
        vkDestroyPipeline(device, vk_pipeline->pipeline, nullptr);
    }

    void VulkanRenderEngine::destroy_texture(Image* resource) {
        VulkanImage* vk_image = static_cast<VulkanImage*>(resource);
        // TODO
        // vmaDestroyImage(vma_allocator, vk_image->image, vk_image->allocation);

        delete vk_image;
    }

    void VulkanRenderEngine::destroy_semaphores(std::vector<Semaphore*>& semaphores) {
        for(Semaphore* semaphore : semaphores) {
            VulkanSemaphore* vk_semaphore = static_cast<VulkanSemaphore*>(semaphore);
            vkDestroySemaphore(device, vk_semaphore->semaphore, nullptr);
        }
    }

    void VulkanRenderEngine::destroy_fences(std::vector<Fence*>& fences) {
        for(Fence* fence : fences) {
            VulkanFence* vk_fence = static_cast<VulkanFence*>(fence);
            vkDestroyFence(device, vk_fence->fence, nullptr);
        }
    }

    CommandList* VulkanRenderEngine::get_command_list(const uint32_t thread_idx,
                                                      const QueueType needed_queue_type,
                                                      const CommandList::Level level) {
        const uint32_t queue_family_index = get_queue_family_index(needed_queue_type);
        const VkCommandPool pool = command_pools_by_thread_idx.at(thread_idx).at(queue_family_index);

        VkCommandBufferAllocateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        create_info.commandPool = pool;
        create_info.level = to_vk_command_buffer_level(level);
        create_info.commandBufferCount = 1;

        VkCommandBuffer new_buffer;
        vkAllocateCommandBuffers(device, &create_info, &new_buffer);

        auto* list = new_object<VulkanCommandList>(new_buffer, this);

        return list;
    }

    void VulkanRenderEngine::submit_command_list(CommandList* cmds,
                                                 const QueueType queue,
                                                 Fence* fence_to_signal,
                                                 const std::vector<Semaphore*>& wait_semaphores,
                                                 const std::vector<Semaphore*>& signal_semaphores) {
        VulkanCommandList* vk_list = static_cast<VulkanCommandList*>(cmds);
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

        std::vector<VkSemaphore> vk_wait_semaphores;
        vk_wait_semaphores.reserve(wait_semaphores.size());
        for(const Semaphore* semaphore : wait_semaphores) {
            const VulkanSemaphore* vk_semaphore = static_cast<const VulkanSemaphore*>(semaphore);
            vk_wait_semaphores.push_back(vk_semaphore->semaphore);
        }

        std::vector<VkSemaphore> vk_signal_semaphores;
        vk_signal_semaphores.reserve(signal_semaphores.size());
        for(const Semaphore* semaphore : signal_semaphores) {
            const VulkanSemaphore* vk_semaphore = static_cast<const VulkanSemaphore*>(semaphore);
            vk_signal_semaphores.push_back(vk_semaphore->semaphore);
        }

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = static_cast<uint32_t>(vk_wait_semaphores.size());
        submit_info.pWaitSemaphores = vk_wait_semaphores.data();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &vk_list->cmds;
        submit_info.signalSemaphoreCount = static_cast<uint32_t>(vk_signal_semaphores.size());
        submit_info.pSignalSemaphores = vk_signal_semaphores.data();

        const VulkanFence* vk_fence = static_cast<const VulkanFence*>(fence_to_signal);

        vkQueueSubmit(queue_to_submit_to, 1, &submit_info, vk_fence->fence);
    }

    uint32_t VulkanRenderEngine::get_queue_family_index(const QueueType type) const { return queue_family_indices.at(type); }

    void VulkanRenderEngine::open_window_and_create_surface(const NovaSettings::WindowOptions& options) {
#ifdef NOVA_LINUX
        window = std::make_unique<X11Window>(options);

        VkXlibSurfaceCreateInfoKHR x_surface_create_info;
        x_surface_create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        x_surface_create_info.pNext = nullptr;
        x_surface_create_info.flags = 0;

        auto* x11_window_ptr = dynamic_cast<X11Window*>(window.get());
        x_surface_create_info.dpy = x11_window_ptr->get_display();
        x_surface_create_info.window = x11_window_ptr->get_x11_window();

        NOVA_CHECK_RESULT(vkCreateXlibSurfaceKHR(instance, &x_surface_create_info, nullptr, &surface));

#elif defined(NOVA_WINDOWS)
        window = std::make_unique<Win32Window>(options);

        VkWin32SurfaceCreateInfoKHR win32_surface_create = {};
        win32_surface_create.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        Window* window_ptr = window.get();
        auto* win32_window_ptr = static_cast<Win32Window*>(window_ptr);
        win32_surface_create.hwnd = win32_window_ptr->get_window_handle();

        NOVA_CHECK_RESULT(vkCreateWin32SurfaceKHR(instance, &win32_surface_create, nullptr, &surface));

#else
#error Unsuported window system
#endif
    }

    void VulkanRenderEngine::create_instance() {
        const auto& version = settings.settings.vulkan.application_version;

        VkApplicationInfo application_info;
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pNext = nullptr;
        application_info.pApplicationName = settings.settings.vulkan.application_name;
        application_info.applicationVersion = VK_MAKE_VERSION(version.major, version.minor, version.patch);
        application_info.pEngineName = "Nova Renderer 0.9";
        application_info.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.pApplicationInfo = &application_info;

        if(settings.settings.debug.enabled && settings.settings.debug.enable_validation_layers) {
            enabled_layer_names.push_back("VK_LAYER_LUNARG_standard_validation");
            // enabled_layer_names.push_back("VK_LAYER_LUNARG_api_dump");
        }
        create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layer_names.size());
        create_info.ppEnabledLayerNames = enabled_layer_names.data();

        std::vector<const char*> enabled_extension_names;
        enabled_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef NOVA_LINUX
        enabled_extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(NOVA_WINDOWS)
        enabled_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
#error Unsupported Operating system
#endif

        if(settings.settings.debug.enabled) {
            enabled_extension_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            enabled_extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extension_names.size());
        create_info.ppEnabledExtensionNames = enabled_extension_names.data();

        NOVA_CHECK_RESULT(vkCreateInstance(&create_info, nullptr, &instance));
    }

    void VulkanRenderEngine::enable_debug_output() {
        vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {};
        debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_create_info.pNext = nullptr;
        debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_create_info.pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(&debug_report_callback);
        debug_create_info.pUserData = this;

        NOVA_CHECK_RESULT(vkCreateDebugUtilsMessengerEXT(instance, &debug_create_info, nullptr, &debug_callback));
    }

    void VulkanRenderEngine::create_device_and_queues() {
        uint32_t device_count;
        NOVA_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));
        auto physical_devices = std::vector<VkPhysicalDevice>(device_count);
        NOVA_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));

        uint32_t graphics_family_idx = 0xFFFFFFFF;
        uint32_t compute_family_idx = 0xFFFFFFFF;
        uint32_t copy_family_idx = 0xFFFFFFFF;

        for(uint32_t device_idx = 0; device_idx < device_count; device_idx++) {
            graphics_family_idx = 0xFFFFFFFF;
            // NOLINTNEXTLINE(misc-misplaced-const)
            const VkPhysicalDevice current_device = physical_devices[device_idx];
            vkGetPhysicalDeviceProperties(current_device, &gpu.props);

            if(gpu.props.vendorID == 0x8086 &&
               device_count - 1 > device_idx) { // Intel GPU... they are not powerful and we have more available, so skip it
                continue;
            }

            if(!does_device_support_extensions(current_device)) {
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
                NOVA_LOG(INFO) << fmt::format(fmt("Selected GPU {:s}"), gpu.props.deviceName);
                gpu.phys_device = current_device;
                break;
            }
        }

        if(gpu.phys_device == nullptr) {
            NOVA_LOG(ERROR) << "Failed to find good GPU";
            return;
        }

        vkGetPhysicalDeviceFeatures(gpu.phys_device, &gpu.supported_features);

        vkGetPhysicalDeviceMemoryProperties(gpu.phys_device, &gpu.memory_properties);

        const float priority = 1.0;

        VkDeviceQueueCreateInfo graphics_queue_create_info{};
        graphics_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphics_queue_create_info.pNext = nullptr;
        graphics_queue_create_info.flags = 0;
        graphics_queue_create_info.queueCount = 1;
        graphics_queue_create_info.queueFamilyIndex = graphics_family_idx;
        graphics_queue_create_info.pQueuePriorities = &priority;

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos = {graphics_queue_create_info};

        VkPhysicalDeviceFeatures physical_device_features{};
        physical_device_features.geometryShader = VK_TRUE;
        physical_device_features.tessellationShader = VK_TRUE;
        physical_device_features.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pNext = nullptr;
        device_create_info.flags = 0;
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.pEnabledFeatures = &physical_device_features;
        device_create_info.enabledExtensionCount = 1;
        const char* swapchain_extension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        device_create_info.ppEnabledExtensionNames = &swapchain_extension;
        device_create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layer_names.size());
        if(!enabled_layer_names.empty()) {
            device_create_info.ppEnabledLayerNames = enabled_layer_names.data();
        }

        NOVA_CHECK_RESULT(vkCreateDevice(gpu.phys_device, &device_create_info, nullptr, &device));

        graphics_family_index = graphics_family_idx;
        vkGetDeviceQueue(device, graphics_family_idx, 0, &graphics_queue);
        compute_family_index = compute_family_idx;
        vkGetDeviceQueue(device, compute_family_idx, 0, &compute_queue);
        transfer_family_index = copy_family_idx;
        vkGetDeviceQueue(device, copy_family_idx, 0, &copy_queue);
    }

    bool VulkanRenderEngine::does_device_support_extensions(VkPhysicalDevice device) {
        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> available(extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available.data());

        std::set<std::string> required = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        for(const auto& extension : available) {
            required.erase(static_cast<const char*>(extension.extensionName));
        }

        return required.empty();
    }

    void VulkanRenderEngine::create_swapchain() {
        // Check what formats our rendering supports, and create a swapchain with one of those formats

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.phys_device, surface, &gpu.surface_capabilities);

        uint32_t num_surface_formats;
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.phys_device, surface, &num_surface_formats, nullptr);
        gpu.surface_formats.resize(num_surface_formats);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.phys_device, surface, &num_surface_formats, gpu.surface_formats.data());

        uint32_t num_surface_present_modes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.phys_device, surface, &num_surface_present_modes, nullptr);
        std::vector<VkPresentModeKHR> present_modes(num_surface_present_modes);
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.phys_device, surface, &num_surface_present_modes, present_modes.data());

        swapchain = new VulkanSwapchain(NUM_IN_FLIGHT_FRAMES, *this, window->get_window_size(), present_modes);

        swapchain_size = window->get_window_size();
    }

    void VulkanRenderEngine::create_per_thread_command_pools() {
        const uint32_t num_threads = 1;
        command_pools_by_thread_idx.reserve(num_threads);

        for(uint32_t i = 0; i < num_threads; i++) {
            command_pools_by_thread_idx.push_back(make_new_command_pools());
        }
    }

    std::unordered_map<uint32_t, VkCommandPool> VulkanRenderEngine::make_new_command_pools() const {
        std::vector<uint32_t> queue_indices;
        queue_indices.push_back(graphics_family_index);
        queue_indices.push_back(transfer_family_index);
        queue_indices.push_back(compute_family_index);

        std::unordered_map<uint32_t, VkCommandPool> pools_by_queue;
        pools_by_queue.reserve(3);

        for(const uint32_t queue_index : queue_indices) {
            VkCommandPoolCreateInfo command_pool_create_info;
            command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            command_pool_create_info.pNext = nullptr;
            command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            command_pool_create_info.queueFamilyIndex = queue_index;

            VkCommandPool command_pool;
            NOVA_CHECK_RESULT(vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool));
            pools_by_queue[queue_index] = command_pool;
        }

        return pools_by_queue;
    }

    uint32_t VulkanRenderEngine::find_memory_type_with_flags(const uint32_t search_flags, const MemorySearchMode search_mode) const {
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

    std::vector<VkDescriptorSetLayout> VulkanRenderEngine::create_descriptor_set_layouts(
        const std::unordered_map<std::string, ResourceBindingDescription>& all_bindings) const {

        /*
         * A few tasks to accomplish:
         * - Take the unordered map of descriptor sets (all_bindings) and convert it into
         *      VkDescriptorSetLayoutCreateInfo structs, ordering everything along the way
         * -
         */

        std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindings_by_set;
        bindings_by_set.resize(all_bindings.size());

        for(const auto& named_binding : all_bindings) {
            const ResourceBindingDescription& binding = named_binding.second;
            if(binding.set >= bindings_by_set.size()) {
                NOVA_LOG(ERROR) << "You've skipped one or more descriptor sets! Don't do that, Nova can't handle it";
                continue;
            }

            VkDescriptorSetLayoutBinding descriptor_binding = {};
            descriptor_binding.binding = binding.binding;
            descriptor_binding.descriptorType = to_vk_descriptor_type(binding.type);
            descriptor_binding.descriptorCount = binding.count;
            descriptor_binding.stageFlags = to_vk_shader_stage_flags(binding.stages);

            bindings_by_set[binding.set].push_back(descriptor_binding);
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

    VkImageView VulkanRenderEngine::image_view_for_image(const Image* image) { 
        // TODO: This method is terrible. We shouldn't tie image views to images, we should let everything that wants
        // to use the image create its own image view
        
        const auto* vk_image = static_cast<const VulkanImage*>(image);

        return vk_image->image_view;
    }

    VkCommandBufferLevel VulkanRenderEngine::to_vk_command_buffer_level(const CommandList::Level level) {
        switch(level) {
            case CommandList::Level::Primary:
                return VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            case CommandList::Level::Secondary:
                return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        }

        return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderEngine::debug_report_callback(const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                                             const VkDebugUtilsMessageTypeFlagsEXT message_types,
                                                                             const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                                             void* /* pUserData */) {
        std::string type = "General";
        if((message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0U) {
            type = "Validation";
        } else if((message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0U) {
            type = "Performance";
        }

        std::stringstream ss;
        ss << "[" << type << "]";
        if(callback_data->queueLabelCount != 0) {
            ss << " Queues: ";
            for(uint32_t i = 0; i < callback_data->queueLabelCount; i++) {
                ss << callback_data->pQueueLabels[i].pLabelName;
                if(i != callback_data->queueLabelCount - 1) {
                    ss << ", ";
                }
            }
        }

        if(callback_data->cmdBufLabelCount != 0) {
            ss << " Command Buffers: ";
            for(uint32_t i = 0; i < callback_data->cmdBufLabelCount; i++) {
                ss << callback_data->pCmdBufLabels[i].pLabelName;
                if(i != callback_data->cmdBufLabelCount - 1) {
                    ss << ", ";
                }
            }
        }

        if(callback_data->objectCount != 0) {
            ss << " Objects: ";
            for(uint32_t i = 0; i < callback_data->objectCount; i++) {
                ss << to_string(callback_data->pObjects[i].objectType);
                if(callback_data->pObjects[i].pObjectName != nullptr) {
                    ss << callback_data->pObjects[i].pObjectName;
                }
                ss << " (" << callback_data->pObjects[i].objectHandle << ") ";
                if(i != callback_data->objectCount - 1) {
                    ss << ", ";
                }
            }
        }

        if(callback_data->pMessage != nullptr) {
            ss << callback_data->pMessage;
        }

        const std::string msg = ss.str();

        if((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
            NOVA_LOG(ERROR) << "[" << type << "] " << msg;

        } else if((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
            // Warnings may hint at unexpected / non-spec API usage
            NOVA_LOG(WARN) << "[" << type << "] " << msg;

        } else if(((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) &&
                  ((message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) == 0U)) { // No validation info!
            // Informal messages that may become handy during debugging
            NOVA_LOG(INFO) << "[" << type << "] " << msg;

        } else if((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) != 0) {
            // Diagnostic info from the Vulkan loader and layers
            // Usually not helpful in terms of API usage, but may help to debug layer and loader problems
            NOVA_LOG(DEBUG) << "[" << type << "] " << msg;

        } else if((message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) == 0U) { // No validation info!
            // Catch-all to be super sure
            NOVA_LOG(INFO) << "[" << type << "]" << msg;
        }

#ifdef NOVA_LINUX
        if((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
            nova_backtrace();
        }
#endif
        return VK_FALSE;
    }

    VkShaderModule VulkanRenderEngine::create_shader_module(const std::vector<uint32_t>& spirv) const {
        VkShaderModuleCreateInfo shader_module_create_info;
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.pNext = nullptr;
        shader_module_create_info.flags = 0;
        shader_module_create_info.pCode = spirv.data();
        shader_module_create_info.codeSize = spirv.size() * 4;

        VkShaderModule module;
        NOVA_CHECK_RESULT(vkCreateShaderModule(device, &shader_module_create_info, nullptr, &module));

        return module;
    }
} // namespace nova::renderer::rhi
