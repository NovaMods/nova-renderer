// This define MUST be before including vulkan_render_device.hpp
#define VMA_IMPLEMENTATION
#include "vulkan_render_device.hpp"

#include <sstream>

#include <rx/core/log.h>
#include <rx/core/set.h>
#include <signal.h>
#include <string.h>

#include "nova_renderer/constants.hpp"
#include "nova_renderer/memory/allocation_structs.hpp"
#include "nova_renderer/renderables.hpp"
#include "nova_renderer/window.hpp"

#include "rx/core/algorithm/max.h"
#include "vk_structs.hpp"
#include "vulkan_command_list.hpp"
#include "vulkan_utils.hpp"
// TODO: Move window creation out of the RHI
#ifdef NOVA_LINUX
#define NOVA_VK_XLIB
#include "../../util/linux_utils.hpp"

#elif defined(NOVA_WINDOWS)
#include "nova_renderer/util/windows.hpp"

#endif

using namespace nova::mem;

namespace nova::renderer::rhi {
    RX_LOG("VulkanRenderDevice", logger);

    VulkanRenderDevice::VulkanRenderDevice(NovaSettingsAccessManager& settings, NovaWindow& window, rx::memory::allocator* allocator)
        : RenderDevice{settings, window, allocator},
          vk_internal_allocator{wrap_allocator(internal_allocator)},
          command_pools_by_thread_idx{internal_allocator} {
        create_instance();

        if(settings.settings.debug.enabled) {
            enable_debug_output();
        }

        create_surface();

        create_device_and_queues();

        save_device_info();

        initialize_vma();

        if(settings.settings.debug.enabled) {
            // Late init, can only be used when the device has already been created
            vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
                vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));
        }

        create_swapchain();

        create_per_thread_command_pools();
    }

    void VulkanRenderDevice::set_num_renderpasses(uint32_t /* num_renderpasses */) {
        // Pretty sure Vulkan doesn't need to do anything here
    }

    ntl::Result<DeviceMemory*> VulkanRenderDevice::allocate_device_memory(const Bytes size,
                                                                          const MemoryUsage usage,
                                                                          const ObjectType /* allowed_objects */,
                                                                          rx::memory::allocator* allocator) {
        auto* memory = allocator->create<VulkanDeviceMemory>();

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = size.b_count();
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

        auto vk_alloc = wrap_allocator(allocator);
        vkAllocateMemory(device, &alloc_info, &vk_alloc, &memory->memory);

        if(usage == MemoryUsage::StagingBuffer) {
            void* mapped_memory;
            vkMapMemory(device, memory->memory, 0, VK_WHOLE_SIZE, 0, &mapped_memory);
            heap_mappings.insert(memory->memory, mapped_memory);
        }

        return ntl::Result<DeviceMemory*>(memory);
    }

    ntl::Result<Renderpass*> VulkanRenderDevice::create_renderpass(const shaderpack::RenderPassCreateInfo& data,
                                                                   const glm::uvec2& framebuffer_size,
                                                                   rx::memory::allocator* allocator) {
        auto* vk_swapchain = static_cast<VulkanSwapchain*>(swapchain);
        VkExtent2D swapchain_extent = {swapchain_size.x, swapchain_size.y};

        auto* renderpass = allocator->create<VulkanRenderpass>();

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
        data.texture_outputs.each_fwd([&](const shaderpack::TextureAttachmentInfo& attachment) {
            if(attachment.name == BACKBUFFER_NAME) {
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

            } else {
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
            return ntl::Result<Renderpass*>(MAKE_ERROR(
                "Framebuffer width for pass {:s} is 0. This is illegal! Make sure that there is at least one attachment for this render pass, and ensure that all attachments used by this pass have a non-zero width",
                data.name.data()));
        }

        if(framebuffer_height == 0) {
            return ntl::Result<Renderpass*>(MAKE_ERROR(
                "Framebuffer height for pass {:s} is 0. This is illegal! Make sure that there is at least one attachment for this render pass, and ensure that all attachments used by this pass have a non-zero height",
                data.name.data()));
        }

        if(framebuffer_attachments.size() > gpu.props.limits.maxColorAttachments) {
            return ntl::Result<Renderpass*>(MAKE_ERROR(
                "Framebuffer for pass {:s} has {:d} color attachments, but your GPU only supports {:d}. Please reduce the number of attachments that this pass uses, possibly by changing some of your input attachments to bound textures",
                data.name.data(),
                data.texture_outputs.size(),
                gpu.props.limits.maxColorAttachments));
        }

        subpass_description.colorAttachmentCount = static_cast<uint32_t>(attachment_references.size());
        subpass_description.pColorAttachments = attachment_references.data();

        render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_create_info.pAttachments = attachments.data();

        auto vk_alloc = wrap_allocator(allocator);
        NOVA_CHECK_RESULT(vkCreateRenderPass(device, &render_pass_create_info, &vk_alloc, &renderpass->pass));

        if(writes_to_backbuffer) {
            if(data.texture_outputs.size() > 1) {
                logger(
                    rx::log::level::k_error,
                    "Pass %s writes to the backbuffer, and other textures. Passes that write to the backbuffer are not allowed to write to any other textures",
                    data.name);
            }
        }

        renderpass->render_area = {{0, 0}, {framebuffer_width, framebuffer_height}};

        if(settings.settings.debug.enabled) {
            VkDebugUtilsObjectNameInfoEXT object_name = {};
            object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            object_name.objectType = VK_OBJECT_TYPE_RENDER_PASS;
            object_name.objectHandle = reinterpret_cast<uint64_t>(renderpass->pass);
            object_name.pObjectName = data.name.data();
            NOVA_CHECK_RESULT(vkSetDebugUtilsObjectNameEXT(device, &object_name));
        }

        return ntl::Result(static_cast<Renderpass*>(renderpass));
    }

    Framebuffer* VulkanRenderDevice::create_framebuffer(const Renderpass* renderpass,
                                                        const rx::vector<Image*>& color_attachments,
                                                        const rx::optional<Image*> depth_attachment,
                                                        const glm::uvec2& framebuffer_size,
                                                        rx::memory::allocator* allocator) {
        const auto* vk_renderpass = static_cast<const VulkanRenderpass*>(renderpass);

        rx::vector<VkImageView> attachment_views(allocator);
        attachment_views.reserve(color_attachments.size() + 1);

        color_attachments.each_fwd([&](const Image* attachment) {
            const auto* vk_image = static_cast<const VulkanImage*>(attachment);
            attachment_views.push_back(vk_image->image_view);
        });

        // Depth attachment is ALWAYS the last attachment
        if(depth_attachment) {
            const auto* vk_depth_image = static_cast<const VulkanImage*>(*depth_attachment);
            attachment_views.push_back(vk_depth_image->image_view);
        }

        VkFramebufferCreateInfo framebuffer_create_info = {};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = vk_renderpass->pass;
        framebuffer_create_info.attachmentCount = static_cast<uint32_t>(attachment_views.size());
        framebuffer_create_info.pAttachments = attachment_views.data();
        framebuffer_create_info.width = framebuffer_size.x;
        framebuffer_create_info.height = framebuffer_size.y;
        framebuffer_create_info.layers = 1;

        auto* framebuffer = allocator->create<VulkanFramebuffer>();
        framebuffer->size = framebuffer_size;
        framebuffer->num_attachments = static_cast<uint32_t>(attachment_views.size());

        auto vk_alloc = wrap_allocator(allocator);
        NOVA_CHECK_RESULT(vkCreateFramebuffer(device, &framebuffer_create_info, &vk_alloc, &framebuffer->framebuffer));

        return framebuffer;
    }

    ntl::Result<PipelineInterface*> VulkanRenderDevice::create_pipeline_interface(
        const rx::map<rx::string, ResourceBindingDescription>& bindings,
        const rx::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
        const rx::optional<shaderpack::TextureAttachmentInfo>& depth_texture,
        rx::memory::allocator* allocator) {

        auto* vk_swapchain = static_cast<VulkanSwapchain*>(swapchain);
        auto* pipeline_interface = allocator->create<VulkanPipelineInterface>();
        pipeline_interface->bindings = bindings;

        pipeline_interface->layouts_by_set = create_descriptor_set_layouts(bindings,
                                                                           pipeline_interface->variable_descriptor_set_counts,
                                                                           allocator);

        VkPipelineLayoutCreateInfo pipeline_layout_create_info;
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.pNext = nullptr;
        pipeline_layout_create_info.flags = 0;
        pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(pipeline_interface->layouts_by_set.size());
        pipeline_layout_create_info.pSetLayouts = pipeline_interface->layouts_by_set.data();
        pipeline_layout_create_info.pushConstantRangeCount = 0;
        pipeline_layout_create_info.pPushConstantRanges = nullptr;

        auto vk_alloc = wrap_allocator(allocator);
        NOVA_CHECK_RESULT(vkCreatePipelineLayout(device, &pipeline_layout_create_info, &vk_alloc, &pipeline_interface->pipeline_layout));

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

        rx::vector<VkAttachmentReference> attachment_references{internal_allocator};
        rx::vector<VkAttachmentDescription> attachment_descriptions{internal_allocator};
        rx::vector<VkImageView> framebuffer_attachments{internal_allocator};

        // Collect framebuffer size information from color output attachments
        color_attachments.each_fwd([&](const shaderpack::TextureAttachmentInfo& attachment) {
            if(attachment.name == BACKBUFFER_NAME) {
                // Handle backbuffer
                // Backbuffer framebuffers are handled by themselves in their own special snowflake way so we just need to skip
                // everything

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

                attachment_descriptions.push_back(desc);

                VkAttachmentReference ref;

                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                ref.attachment = static_cast<uint32_t>(attachment_descriptions.size()) - 1;

                attachment_references.push_back(ref);

                return false;
            }

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

            attachment_descriptions.push_back(desc);

            VkAttachmentReference ref;

            ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ref.attachment = static_cast<uint32_t>(attachment_descriptions.size()) - 1;

            attachment_references.push_back(ref);

            return true;
        });

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

        NOVA_CHECK_RESULT(vkCreateRenderPass(device, &render_pass_create_info, &vk_alloc, &pipeline_interface->pass));

        return ntl::Result(static_cast<PipelineInterface*>(pipeline_interface));
    }

    DescriptorPool* VulkanRenderDevice::create_descriptor_pool(const rx::map<DescriptorType, uint32_t>& descriptor_capacity,
                                                               rx::memory::allocator* allocator) {
        rx::vector<VkDescriptorPoolSize> pool_sizes{internal_allocator};

        uint32_t max_sets = 0;
        descriptor_capacity.each_pair([&](const DescriptorType& type, const uint32_t count) {
            pool_sizes.emplace_back(VkDescriptorPoolSize{to_vk_descriptor_type(type), count});
            max_sets += count;
        });

        VkDescriptorPoolCreateInfo pool_create_info = {};
        pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_create_info.maxSets = max_sets;
        pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        pool_create_info.pPoolSizes = pool_sizes.data();
        auto* pool = allocator->create<VulkanDescriptorPool>();
        auto vk_alloc = wrap_allocator(allocator);
        NOVA_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_create_info, &vk_alloc, &pool->descriptor_pool));

        return pool;
    }

    rx::vector<DescriptorSet*> VulkanRenderDevice::create_descriptor_sets(const PipelineInterface* pipeline_interface,
                                                                          DescriptorPool* pool,
                                                                          rx::memory::allocator* allocator) {
        const auto* vk_pipeline_interface = static_cast<const VulkanPipelineInterface*>(pipeline_interface);
        const auto* vk_pool = static_cast<const VulkanDescriptorPool*>(pool);

        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = vk_pool->descriptor_pool;
        alloc_info.descriptorSetCount = static_cast<uint32_t>(vk_pipeline_interface->layouts_by_set.size());
        alloc_info.pSetLayouts = vk_pipeline_interface->layouts_by_set.data();

        rx::vector<uint32_t> counts{allocator};
        counts.reserve(vk_pipeline_interface->layouts_by_set.size());

        VkDescriptorSetVariableDescriptorCountAllocateInfo set_counts = {};
        set_counts.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
        set_counts.descriptorSetCount = vk_pipeline_interface->variable_descriptor_set_counts.size();
        set_counts.pDescriptorCounts = vk_pipeline_interface->variable_descriptor_set_counts.data();

        alloc_info.pNext = &set_counts;

        rx::vector<VkDescriptorSet> sets{allocator};
        sets.resize(vk_pipeline_interface->layouts_by_set.size());
        vkAllocateDescriptorSets(device, &alloc_info, sets.data());

        rx::vector<DescriptorSet*> final_sets{allocator};
        final_sets.reserve(sets.size());
        sets.each_fwd([&](const VkDescriptorSet set) {
            auto* vk_set = allocator->create<VulkanDescriptorSet>();
            vk_set->descriptor_set = set;
            final_sets.push_back(vk_set);
        });

        return final_sets;
    }

    void VulkanRenderDevice::update_descriptor_sets(rx::vector<DescriptorSetWrite>& writes) {
        rx::vector<VkWriteDescriptorSet> vk_writes{internal_allocator};
        vk_writes.reserve(writes.size());

        rx::vector<VkDescriptorImageInfo> image_infos{internal_allocator};
        image_infos.reserve(writes.size());

        rx::vector<VkDescriptorBufferInfo> buffer_infos{internal_allocator};
        buffer_infos.reserve(writes.size());

        writes.each_fwd([&](const DescriptorSetWrite& write) {
            VkWriteDescriptorSet vk_write = {};
            vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            vk_write.dstSet = static_cast<const VulkanDescriptorSet*>(write.set)->descriptor_set;
            vk_write.dstBinding = write.binding;
            vk_write.descriptorCount = static_cast<uint32_t>(write.resources.size());
            vk_write.dstArrayElement = 0;

            switch(write.type) {
                case DescriptorType::CombinedImageSampler: {
                    // We have a single array for all the image infos, so we need to save the index of the image info for this descriptor
                    // update
                    const auto first_image_info_idx = image_infos.size();

                    write.resources.each_fwd([&](const DescriptorResourceInfo& info) {
                        VkDescriptorImageInfo vk_image_info;
                        vk_image_info.imageView = image_view_for_image(info.image_info.image);
                        vk_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        vk_image_info.sampler = static_cast<VulkanSampler*>(info.image_info.sampler)->sampler;

                        image_infos.emplace_back(vk_image_info);
                    });

                    vk_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    vk_write.pImageInfo = &image_infos[first_image_info_idx];

                    // logger(rx::log::level::k_verbose, "Updating CombinedImageSampler descriptor set %x", vk_write.dstSet);

                    vk_writes.push_back(vk_write);
                } break;

                case DescriptorType::UniformBuffer: {
                    // We have a single array for all the buffer infos, so we need to save the index of the buffer info for this descriptor
                    // update
                    const auto first_buffer_info_idx = buffer_infos.size();

                    write.resources.each_fwd([&](const DescriptorResourceInfo& info) {
                        const auto* vk_buffer = static_cast<const VulkanBuffer*>(info.buffer_info.buffer);

                        VkDescriptorBufferInfo vk_buffer_info;
                        vk_buffer_info.buffer = vk_buffer->buffer;
                        vk_buffer_info.offset = vk_buffer->memory.allocation_info.offset.b_count();
                        vk_buffer_info.range = vk_buffer->size.b_count();

                        buffer_infos.emplace_back(vk_buffer_info);
                    });

                    vk_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    vk_write.pBufferInfo = &buffer_infos[first_buffer_info_idx];

                    // logger(rx::log::level::k_verbose, "Updating UniformBuffer descriptor set %x", vk_write.dstSet);

                    vk_writes.push_back(vk_write);
                } break;

                case DescriptorType::StorageBuffer: {
                    // We have a single array for all the buffer infos, so we need to save the index of the buffer info for this descriptor
                    // update
                    const auto first_buffer_info_idx = buffer_infos.size();
                    write.resources.each_fwd([&](const DescriptorResourceInfo& info) {
                        const auto* vk_buffer = static_cast<const VulkanBuffer*>(info.buffer_info.buffer);

                        VkDescriptorBufferInfo vk_buffer_info;
                        vk_buffer_info.buffer = vk_buffer->buffer;
                        vk_buffer_info.offset = vk_buffer->memory.allocation_info.offset.b_count();
                        vk_buffer_info.range = vk_buffer->size.b_count();

                        buffer_infos.emplace_back(vk_buffer_info);
                    });

                    vk_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    vk_write.pBufferInfo = &buffer_infos[first_buffer_info_idx];

                    // logger(rx::log::level::k_verbose, "Updating StorageBuffer descriptor set %x", vk_write.dstSet);

                    vk_writes.push_back(vk_write);
                } break;

                case DescriptorType::Texture: {
                    // We have a single array for all the image infos, so we need to save the index of the image info for this descriptor
                    // update
                    const auto first_image_info_idx = image_infos.size();

                    write.resources.each_fwd([&](const DescriptorResourceInfo& info) {
                        VkDescriptorImageInfo vk_image_info = {};
                        vk_image_info.imageView = image_view_for_image(info.image_info.image);
                        vk_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                        image_infos.emplace_back(vk_image_info);
                    });

                    vk_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                    vk_write.pImageInfo = &image_infos[first_image_info_idx];

                    // logger(rx::log::level::k_verbose, "Updating Texture descriptor set %x", vk_write.dstSet);

                    vk_writes.push_back(vk_write);
                } break;

                case DescriptorType::Sampler: {
                    // We have a single array for all the image infos, so we need to save the index of the image info for this descriptor
                    // update
                    // Also we store the sampler infos in the same array as the image infos and it's weird
                    const auto first_image_info_idx = image_infos.size();

                    write.resources.each_fwd([&](const DescriptorResourceInfo& info) {
                        VkDescriptorImageInfo vk_image_info = {};
                        vk_image_info.sampler = static_cast<VulkanSampler*>(info.sampler_info.sampler)->sampler;

                        image_infos.emplace_back(vk_image_info);
                    });

                    vk_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                    vk_write.pImageInfo = &image_infos[first_image_info_idx];

                    // logger(rx::log::level::k_verbose, "Updating Sampler descriptor set %x", vk_write.dstSet);

                    vk_writes.push_back(vk_write);

                } break;

                default:
                    logger(rx::log::level::k_error,
                           "Don't know how to update %s descriptor set %x",
                           descriptor_type_to_string(write.type),
                           vk_write.dstSet);
            }
        });

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(vk_writes.size()), vk_writes.data(), 0, nullptr);
    }

    void VulkanRenderDevice::reset_descriptor_pool(DescriptorPool* pool) {
        auto* vk_pool = static_cast<VulkanDescriptorPool*>(pool);
        vkResetDescriptorPool(device, vk_pool->descriptor_pool, 0);
    }

    ntl::Result<Pipeline*> VulkanRenderDevice::create_pipeline(PipelineInterface* pipeline_interface,
                                                               const shaderpack::PipelineCreateInfo& data,
                                                               rx::memory::allocator* allocator) {
        logger(rx::log::level::k_verbose, "Creating a VkPipeline for pipeline %s", data.name);

        const auto* vk_interface = static_cast<const VulkanPipelineInterface*>(pipeline_interface);
        auto* vk_pipeline = allocator->create<VulkanPipeline>();

        rx::vector<VkPipelineShaderStageCreateInfo> shader_stages(internal_allocator);
        rx::map<VkShaderStageFlags, VkShaderModule> shader_modules(internal_allocator);

        logger(rx::log::level::k_verbose, "Compiling vertex module");
        const auto vertex_module = create_shader_module(data.vertex_shader.source);
        if(vertex_module) {
            shader_modules.insert(VK_SHADER_STAGE_VERTEX_BIT, *vertex_module);
        } else {
            return ntl::Result<Pipeline*>(ntl::NovaError("Could not create vertex module"));
        }

        if(data.geometry_shader) {
            logger(rx::log::level::k_verbose, "Compiling geometry module");
            const auto geometry_module = create_shader_module(data.geometry_shader->source);
            if(geometry_module) {
                shader_modules.insert(VK_SHADER_STAGE_GEOMETRY_BIT, *geometry_module);
            } else {
                return ntl::Result<Pipeline*>(ntl::NovaError("Could not geometry vertex module"));
            }
        }

        if(data.tessellation_control_shader) {
            logger(rx::log::level::k_verbose, "Compiling tessellation_control module");
            const auto tessellation_control_module = create_shader_module(data.tessellation_control_shader->source);
            if(tessellation_control_module) {
                shader_modules.insert(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, *tessellation_control_module);
            } else {
                return ntl::Result<Pipeline*>(ntl::NovaError("Could not geometry vertex module"));
            }
        }

        if(data.tessellation_evaluation_shader) {
            logger(rx::log::level::k_verbose, "Compiling tessellation_evaluation module");
            const auto tessellation_evaluation_module = create_shader_module(data.tessellation_evaluation_shader->source);
            if(tessellation_evaluation_module) {
                shader_modules.insert(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, *tessellation_evaluation_module);
            } else {
                return ntl::Result<Pipeline*>(ntl::NovaError("Could not geometry vertex module"));
            }
        }

        if(data.fragment_shader) {
            logger(rx::log::level::k_verbose, "Compiling fragment module");
            const auto fragment_module = create_shader_module(data.fragment_shader->source);
            if(fragment_module) {
                shader_modules.insert(VK_SHADER_STAGE_FRAGMENT_BIT, *fragment_module);
            } else {
                return ntl::Result<Pipeline*>(ntl::NovaError("Could not geometry vertex module"));
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

        const auto& [vertex_attribute_descriptions, vertex_binding_descriptions] = get_input_assembler_setup(vk_interface->vertex_fields);

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

        const auto npos = rx::vector<shaderpack::StateEnum>::k_npos;

        VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
        depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_create_info.depthTestEnable = static_cast<VkBool32>(data.states.find(shaderpack::StateEnum::DisableDepthTest) ==
                                                                          npos);

        depth_stencil_create_info.depthWriteEnable = static_cast<VkBool32>(data.states.find(shaderpack::StateEnum::DisableDepthWrite) ==
                                                                           npos);

        depth_stencil_create_info.depthCompareOp = to_compare_op(data.depth_func);
        depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
        depth_stencil_create_info.stencilTestEnable = static_cast<VkBool32>(data.states.find(shaderpack::StateEnum::EnableStencilTest) !=
                                                                            npos);

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

        const auto should_blend = data.states.find(shaderpack::StateEnum::Blending) != npos;

        VkPipelineColorBlendAttachmentState color_blend_attachment;
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                                VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_TRUE;
        color_blend_attachment.srcColorBlendFactor = to_blend_factor(data.source_color_blend_factor);
        color_blend_attachment.dstColorBlendFactor = to_blend_factor(data.destination_color_blend_factor);
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment.srcAlphaBlendFactor = to_blend_factor(data.source_alpha_blend_factor);
        color_blend_attachment.dstAlphaBlendFactor = to_blend_factor(data.destination_alpha_blend_factor);
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

        rx::vector<VkDynamicState> dynamic_states;

        if(data.scissor_mode == shaderpack::ScissorTestMode::DynamicScissorRect) {
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
        if(should_blend) {
            pipeline_create_info.pColorBlendState = &color_blend_create_info;
        }
        pipeline_create_info.pDynamicState = &dynamic_state_create_info;
        pipeline_create_info.layout = vk_interface->pipeline_layout;

        pipeline_create_info.renderPass = vk_interface->pass;
        pipeline_create_info.subpass = 0;
        pipeline_create_info.basePipelineIndex = -1;

        auto vk_alloc = wrap_allocator(allocator);
        VkResult result = vkCreateGraphicsPipelines(device, nullptr, 1, &pipeline_create_info, &vk_alloc, &vk_pipeline->pipeline);
        if(result != VK_SUCCESS) {
            return ntl::Result<Pipeline*>(MAKE_ERROR("Could not compile pipeline {:s}", data.name.data()));
        }

        if(settings.settings.debug.enabled) {
            VkDebugUtilsObjectNameInfoEXT object_name = {};
            object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            object_name.objectType = VK_OBJECT_TYPE_PIPELINE;
            object_name.objectHandle = reinterpret_cast<uint64_t>(vk_pipeline->pipeline);
            object_name.pObjectName = data.name.data();
            NOVA_CHECK_RESULT(vkSetDebugUtilsObjectNameEXT(device, &object_name));
            logger(rx::log::level::k_info, "Set pipeline %u to have name %s", vk_pipeline->pipeline, data.name);
        }

        return ntl::Result(static_cast<Pipeline*>(vk_pipeline));
    } // namespace nova::renderer::rhi

    Buffer* VulkanRenderDevice::create_buffer(const BufferCreateInfo& info,
                                              DeviceMemoryResource& memory,
                                              rx::memory::allocator* allocator) {
        auto* buffer = allocator->create<VulkanBuffer>();

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
                vma_alloc.usage = VMA_MEMORY_USAGE_GPU_ONLY;
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

        const auto result = vmaCreateBuffer(vma, &vk_create_info, &vma_alloc, &buffer->buffer, &buffer->allocation, nullptr);
        if(result == VK_SUCCESS) {
            logger(rx::log::level::k_verbose, "Created buffer %s with size %u", info.name, vk_create_info.size);

            buffer->size = info.size;

            if(settings->debug.enabled) {
                VkDebugUtilsObjectNameInfoEXT object_name = {};
                object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                object_name.objectType = VK_OBJECT_TYPE_BUFFER;
                object_name.objectHandle = reinterpret_cast<uint64_t>(buffer->buffer);
                object_name.pObjectName = info.name.data();

                NOVA_CHECK_RESULT(vkSetDebugUtilsObjectNameEXT(device, &object_name));

                logger(rx::log::level::k_info, "Set buffer %uz to have name %s", buffer->buffer, info.name);
            }

            return buffer;

        } else {
            logger(rx::log::level::k_error, "Could not create buffer %s: %s", info.name, to_string(result));

            return nullptr;
        }
    }

    void VulkanRenderDevice::write_data_to_buffer(const void* data,
                                                  const mem::Bytes num_bytes,
                                                  const mem::Bytes offset,
                                                  const Buffer* buffer) {
        const auto* vulkan_buffer = static_cast<const VulkanBuffer*>(buffer);

        const AllocationInfo& allocation_info = vulkan_buffer->memory.allocation_info;
        const auto* memory = static_cast<const VulkanDeviceMemory*>(vulkan_buffer->memory.memory);

        // TODO: heap_mappings doesn't have the buffer's memory in it
        // Alternately, the buffer's memory isn't in heap_mappings
        // Something something assuming constantly mapped?
        uint8_t* mapped_bytes = static_cast<uint8_t*>(*heap_mappings.find(memory->memory)) + allocation_info.offset.b_count() +
                                offset.b_count();
        memcpy(mapped_bytes, data, num_bytes.b_count());
    }

    Sampler* VulkanRenderDevice::create_sampler(const SamplerCreateInfo& create_info, rx::memory::allocator* allocator) {
        auto* sampler = allocator->create<VulkanSampler>();

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

        auto alloc_calls = wrap_allocator(allocator);
        vkCreateSampler(device, &vk_create_info, &alloc_calls, &sampler->sampler);

        return sampler;
    }

    Image* VulkanRenderDevice::create_image(const shaderpack::TextureCreateInfo& info, rx::memory::allocator* allocator) {
        auto* image = allocator->create<VulkanImage>();

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

        if(info.usage == shaderpack::ImageUsage::SampledImage) {
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
                VkDebugUtilsObjectNameInfoEXT object_name = {};
                object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                object_name.objectType = VK_OBJECT_TYPE_IMAGE;
                object_name.objectHandle = reinterpret_cast<uint64_t>(image->image);
                object_name.pObjectName = info.name.data();

                NOVA_CHECK_RESULT(vkSetDebugUtilsObjectNameEXT(device, &object_name));

                logger(rx::log::level::k_info, "Set image %uz to have name %s", image->image, info.name);
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

            auto vk_alloc = wrap_allocator(allocator);
            vkCreateImageView(device, &image_view_create_info, &vk_alloc, &image->image_view);

            return image;

        } else {
            logger(rx::log::level::k_error, "Could not create image %s: %s", info.name, to_string(result));

            return nullptr;
        }
    }

    Semaphore* VulkanRenderDevice::create_semaphore(rx::memory::allocator* allocator) {
        auto* semaphore = allocator->create<VulkanSemaphore>();

        VkSemaphoreCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        auto vk_alloc = wrap_allocator(allocator);
        vkCreateSemaphore(device, &create_info, &vk_alloc, &semaphore->semaphore);

        return semaphore;
    }

    rx::vector<Semaphore*> VulkanRenderDevice::create_semaphores(const uint32_t num_semaphores, rx::memory::allocator* allocator) {
        auto semaphores = rx::vector<Semaphore*>{allocator};
        semaphores.reserve(num_semaphores);

        for(uint32_t i = 0; i < num_semaphores; i++) {
            semaphores.emplace_back(create_semaphore(allocator));
        }

        return semaphores;
    }

    Fence* VulkanRenderDevice::create_fence(const bool signaled, rx::memory::allocator* allocator) {
        auto* fence = allocator->create<VulkanFence>();

        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        if(signaled) {
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }

        auto vk_alloc = wrap_allocator(allocator);
        vkCreateFence(device, &fence_create_info, &vk_alloc, &fence->fence);

        return fence;
    }

    rx::vector<Fence*> VulkanRenderDevice::create_fences(const uint32_t num_fences, const bool signaled, rx::memory::allocator* allocator) {
        rx::vector<Fence*> fences{allocator};
        fences.reserve(num_fences);

        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        if(signaled) {
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }

        for(uint32_t i = 0; i < num_fences; i++) {
            auto* fence = allocator->create<VulkanFence>();
            auto vk_alloc = wrap_allocator(allocator);
            vkCreateFence(device, &fence_create_info, &vk_alloc, &fence->fence);

            fences.push_back(fence);
        }

        return fences;
    }

    void VulkanRenderDevice::wait_for_fences(const rx::vector<Fence*> fences) {
        rx::vector<VkFence> vk_fences{internal_allocator};
        vk_fences.reserve(fences.size());
        fences.each_fwd([&](const Fence* fence) {
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
                logger(rx::log::level::k_error, "Could not wait for fences. %s (error code %x)", to_string(result), result);
            }
        }
    }

    void VulkanRenderDevice::reset_fences(const rx::vector<Fence*>& fences) {
        rx::vector<VkFence> vk_fences{internal_allocator};
        vk_fences.reserve(fences.size());
        fences.each_fwd([&](const Fence* fence) {
            const auto* vk_fence = static_cast<const VulkanFence*>(fence);
            vk_fences.push_back(vk_fence->fence);
        });

        vkResetFences(device, static_cast<uint32_t>(fences.size()), vk_fences.data());
    }

    void VulkanRenderDevice::destroy_renderpass(Renderpass* pass, rx::memory::allocator* allocator) {
        auto* vk_renderpass = static_cast<VulkanRenderpass*>(pass);
        vkDestroyRenderPass(device, vk_renderpass->pass, nullptr);
        allocator->deallocate(reinterpret_cast<rx_byte*>(pass));
    }

    void VulkanRenderDevice::destroy_framebuffer(Framebuffer* framebuffer, rx::memory::allocator* allocator) {
        const auto* vk_framebuffer = static_cast<const VulkanFramebuffer*>(framebuffer);
        vkDestroyFramebuffer(device, vk_framebuffer->framebuffer, nullptr);

        allocator->deallocate(reinterpret_cast<rx_byte*>(framebuffer));
    }

    void VulkanRenderDevice::destroy_pipeline_interface(PipelineInterface* pipeline_interface, rx::memory::allocator* allocator) {
        auto* vk_interface = static_cast<VulkanPipelineInterface*>(pipeline_interface);
        vkDestroyRenderPass(device, vk_interface->pass, nullptr);
        vkDestroyPipelineLayout(device, vk_interface->pipeline_layout, nullptr);

        allocator->deallocate(reinterpret_cast<rx_byte*>(pipeline_interface));
    }

    void VulkanRenderDevice::destroy_pipeline(Pipeline* pipeline, rx::memory::allocator* allocator) {
        auto* vk_pipeline = static_cast<VulkanPipeline*>(pipeline);
        vkDestroyPipeline(device, vk_pipeline->pipeline, nullptr);

        allocator->deallocate(reinterpret_cast<rx_byte*>(pipeline));
    }

    void VulkanRenderDevice::destroy_texture(Image* resource, rx::memory::allocator* allocator) {
        auto* vk_image = static_cast<VulkanImage*>(resource);
        vmaDestroyImage(vma, vk_image->image, vk_image->allocation);

        allocator->deallocate(reinterpret_cast<rx_byte*>(resource));
    }

    void VulkanRenderDevice::destroy_semaphores(rx::vector<Semaphore*>& semaphores, rx::memory::allocator* allocator) {
        semaphores.each_fwd([&](Semaphore* semaphore) {
            auto* vk_semaphore = static_cast<VulkanSemaphore*>(semaphore);
            vkDestroySemaphore(device, vk_semaphore->semaphore, nullptr);
            allocator->deallocate(reinterpret_cast<rx_byte*>(semaphore));
        });
    }

    void VulkanRenderDevice::destroy_fences(const rx::vector<Fence*>& fences, rx::memory::allocator* allocator) {
        fences.each_fwd([&](Fence* fence) {
            auto* vk_fence = static_cast<VulkanFence*>(fence);
            auto vk_alloc = wrap_allocator(allocator);
            vkDestroyFence(device, vk_fence->fence, &vk_alloc);

            allocator->deallocate(reinterpret_cast<rx_byte*>(fence));
        });
    }

    CommandList* VulkanRenderDevice::create_command_list(const uint32_t thread_idx,
                                                         const QueueType needed_queue_type,
                                                         const CommandList::Level level,
                                                         rx::memory::allocator* allocator) {
        const uint32_t queue_family_index = get_queue_family_index(needed_queue_type);
        const VkCommandPool pool = *command_pools_by_thread_idx[thread_idx].find(queue_family_index);

        VkCommandBufferAllocateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        create_info.commandPool = pool;
        create_info.level = to_vk_command_buffer_level(level);
        create_info.commandBufferCount = 1;

        VkCommandBuffer new_buffer;
        vkAllocateCommandBuffers(device, &create_info, &new_buffer);

        auto* list = allocator->create<VulkanCommandList>(new_buffer, this);

        return list;
    }

    void VulkanRenderDevice::submit_command_list(CommandList* cmds,
                                                 const QueueType queue,
                                                 Fence* fence_to_signal,
                                                 const rx::vector<Semaphore*>& wait_semaphores,
                                                 const rx::vector<Semaphore*>& signal_semaphores) {
        auto* vk_list = static_cast<VulkanCommandList*>(cmds);
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

        rx::vector<VkSemaphore> vk_wait_semaphores(internal_allocator);
        vk_wait_semaphores.reserve(wait_semaphores.size());
        wait_semaphores.each_fwd([&](const Semaphore* semaphore) {
            const auto* vk_semaphore = static_cast<const VulkanSemaphore*>(semaphore);
            vk_wait_semaphores.push_back(vk_semaphore->semaphore);
        });

        rx::vector<VkSemaphore> vk_signal_semaphores(internal_allocator);
        vk_signal_semaphores.reserve(signal_semaphores.size());
        signal_semaphores.each_fwd([&](const Semaphore* semaphore) {
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

        const auto vk_signal_fence = [&] {
            if(fence_to_signal) {
                return static_cast<const VulkanFence*>(fence_to_signal)->fence;

            } else {
                return VkFence{};
            }
        }();

        const auto result = vkQueueSubmit(queue_to_submit_to, 1, &submit_info, vk_signal_fence);

        if(settings->debug.enabled) {
            if(result != VK_SUCCESS) {
                logger(rx::log::level::k_error, "Could submit command list: %s", to_string(result));
            }
        }
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
                return 9999; // I have to return _something_ or Visual Studio gets mad
        }
    }

    void VulkanRenderDevice::create_surface() {
#ifdef NOVA_LINUX
        VkXlibSurfaceCreateInfoKHR x_surface_create_info;
        x_surface_create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        x_surface_create_info.pNext = nullptr;
        x_surface_create_info.flags = 0;

        x_surface_create_info.dpy = window.get_display();
        x_surface_create_info.window = window.get_window_handle();

        auto vk_alloc = wrap_allocator(internal_allocator);
        NOVA_CHECK_RESULT(vkCreateXlibSurfaceKHR(instance, &x_surface_create_info, &vk_alloc, &surface));

#elif defined(NOVA_WINDOWS)
        VkWin32SurfaceCreateInfoKHR win32_surface_create = {};
        win32_surface_create.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32_surface_create.hwnd = window.get_window_handle();

        auto vk_alloc = wrap_allocator(internal_allocator);
        NOVA_CHECK_RESULT(vkCreateWin32SurfaceKHR(instance, &win32_surface_create, &vk_alloc, &surface));

#else
#error Unsuported window system
#endif
    }

    void VulkanRenderDevice::create_instance() {
        const auto& version = settings.settings.vulkan.application_version;

        VkApplicationInfo application_info;
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pNext = nullptr;
        application_info.pApplicationName = settings.settings.vulkan.application_name;
        application_info.applicationVersion = VK_MAKE_VERSION(version.major, version.minor, version.patch);
        application_info.pEngineName = "Nova Renderer 0.9";
        application_info.apiVersion = VK_API_VERSION_1_2;

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

        rx::vector<const char*> enabled_extension_names(internal_allocator);
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

        auto vk_alloc = wrap_allocator(internal_allocator);
        NOVA_CHECK_RESULT(vkCreateInstance(&create_info, &vk_alloc, &instance));
    }

    void VulkanRenderDevice::enable_debug_output() {
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

        auto vk_alloc = wrap_allocator(internal_allocator);
        NOVA_CHECK_RESULT(vkCreateDebugUtilsMessengerEXT(instance, &debug_create_info, &vk_alloc, &debug_callback));
    }

    void VulkanRenderDevice::save_device_info() {
        switch(gpu.props.vendorID) {
            case AMD_PCI_VENDOR_ID:
                info.architecture = DeviceArchitecture::Amd;
                break;

            case INTEL_PCI_VENDOR_ID:
                info.architecture = DeviceArchitecture::Intel;
                break;

            case NVIDIA_PCI_VENDOR_ID:
                info.architecture = DeviceArchitecture::Nvidia;
                break;

            default:
                info.architecture = DeviceArchitecture::Unknown;
        }

        vk_info.max_uniform_buffer_size = gpu.props.limits.maxUniformBufferRange;
        info.max_texture_size = gpu.props.limits.maxImageDimension2D;

        // TODO: Something smarter when Intel releases discreet GPUS
        // TODO: Handle integrated AMD GPUs
        info.is_uma = info.architecture == DeviceArchitecture::Intel;

        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(gpu.phys_device, nullptr, &extension_count, nullptr);
        rx::vector<VkExtensionProperties> available_extensions(internal_allocator, extension_count);
        vkEnumerateDeviceExtensionProperties(gpu.phys_device, nullptr, &extension_count, available_extensions.data());

        const auto extension_name_matcher = [](const char* ext_name) {
            return [=](const VkExtensionProperties& ext_props) -> bool { return strcmp(ext_name, ext_props.extensionName) == 0; };
        };

        // TODO: Update as more GPUs support hardware raytracing
        info.supports_raytracing = available_extensions.find_if(extension_name_matcher(VK_NV_RAY_TRACING_EXTENSION_NAME)) !=
                                   rx::vector<VkExtensionProperties>::k_npos;

        // TODO: Update as more GPUs support mesh shaders
        info.supports_mesh_shaders = available_extensions.find_if(extension_name_matcher(VK_NV_MESH_SHADER_EXTENSION_NAME));
    }

    void VulkanRenderDevice::initialize_vma() {
        VmaAllocatorCreateInfo create_info{};
        create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        create_info.physicalDevice = gpu.phys_device;
        create_info.device = device;
        create_info.pAllocationCallbacks = &vk_internal_allocator;
        create_info.instance = instance;

        const auto result = vmaCreateAllocator(&create_info, &vma);

        if(result != VK_SUCCESS) {
            logger(rx::log::level::k_error, "Could not initialize VMA: %s", to_string(result));
        }
    }

    void VulkanRenderDevice::create_device_and_queues() {
        rx::vector<char*> device_extensions{internal_allocator};
        device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        device_extensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

        uint32_t device_count;
        NOVA_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));
        auto physical_devices = rx::vector<VkPhysicalDevice>{internal_allocator, device_count};
        NOVA_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));

        uint32_t graphics_family_idx = 0xFFFFFFFF;
        uint32_t compute_family_idx = 0xFFFFFFFF;
        uint32_t copy_family_idx = 0xFFFFFFFF;

        for(uint32_t device_idx = 0; device_idx < device_count; device_idx++) {
            graphics_family_idx = 0xFFFFFFFF;
            VkPhysicalDevice current_device = physical_devices[device_idx];
            vkGetPhysicalDeviceProperties(current_device, &gpu.props);

            const bool is_intel_gpu = gpu.props.vendorID == INTEL_PCI_VENDOR_ID;
            const bool more_gpus_available = device_count - 1 > device_idx;
            if(is_intel_gpu && more_gpus_available) {
                // Intel GPU _probably_ isn't as powerful as a discreet GPU, and if there's more than one GPU then the other one(s) are
                // _probably_ discreet GPUs, so let's not use the Intel GPU and instead use the discreet GPU
                // TODO: Make a local device for the integrated GPU when we figure out multi-GPU
                // TODO: Rework this code when Intel releases discreet GPUs
                continue;
            }

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
                logger(rx::log::level::k_info, "Selected GPU %s", gpu.props.deviceName);
                gpu.phys_device = current_device;
                break;
            }
        }

        if(gpu.phys_device == nullptr) {
            logger(rx::log::level::k_error, "Failed to find good GPU");

            // TODO: Message the user that GPU selection failed

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
        device_create_info.pNext = &descriptor_indexing_features;

        auto vk_alloc = wrap_allocator(internal_allocator);
        NOVA_CHECK_RESULT(vkCreateDevice(gpu.phys_device, &device_create_info, &vk_alloc, &device));

        graphics_family_index = graphics_family_idx;
        vkGetDeviceQueue(device, graphics_family_idx, 0, &graphics_queue);
        compute_family_index = compute_family_idx;
        vkGetDeviceQueue(device, compute_family_idx, 0, &compute_queue);
        transfer_family_index = copy_family_idx;
        vkGetDeviceQueue(device, copy_family_idx, 0, &copy_queue);
    }

    bool VulkanRenderDevice::does_device_support_extensions(VkPhysicalDevice device, const rx::vector<char*>& required_device_extensions) {
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

            logger(rx::log::level::k_warning, "Device does not support these required extensions: %s", ss.str().c_str());
        }

        const auto device_supports_required_extensions = required.is_empty();
        return device_supports_required_extensions;
    }

    void VulkanRenderDevice::create_swapchain() {
        // Check what formats our rendering supports, and create a swapchain with one of those formats

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.phys_device, surface, &gpu.surface_capabilities);

        uint32_t num_surface_formats;
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.phys_device, surface, &num_surface_formats, nullptr);
        gpu.surface_formats.resize(num_surface_formats);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.phys_device, surface, &num_surface_formats, gpu.surface_formats.data());

        uint32_t num_surface_present_modes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.phys_device, surface, &num_surface_present_modes, nullptr);
        rx::vector<VkPresentModeKHR> present_modes(internal_allocator, num_surface_present_modes);
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.phys_device, surface, &num_surface_present_modes, present_modes.data());

        swapchain = internal_allocator->create<VulkanSwapchain>(NUM_IN_FLIGHT_FRAMES, this, window.get_framebuffer_size(), present_modes);

        swapchain_size = window.get_framebuffer_size();
    }

    void VulkanRenderDevice::create_per_thread_command_pools() {
        const uint32_t num_threads = 1; // TODO: Make this real
        command_pools_by_thread_idx.reserve(num_threads);

        for(uint32_t i = 0; i < num_threads; i++) {
            command_pools_by_thread_idx.push_back(make_new_command_pools());
        }
    }

    rx::map<uint32_t, VkCommandPool> VulkanRenderDevice::make_new_command_pools() const {
        rx::vector<uint32_t> queue_indices(internal_allocator);
        queue_indices.push_back(graphics_family_index);
        queue_indices.push_back(transfer_family_index);
        queue_indices.push_back(compute_family_index);

        rx::map<uint32_t, VkCommandPool> pools_by_queue(internal_allocator);

        queue_indices.each_fwd([&](const uint32_t queue_index) {
            VkCommandPoolCreateInfo command_pool_create_info;
            command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            command_pool_create_info.pNext = nullptr;
            command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            command_pool_create_info.queueFamilyIndex = queue_index;

            auto vk_alloc = wrap_allocator(internal_allocator);
            VkCommandPool command_pool;
            NOVA_CHECK_RESULT(vkCreateCommandPool(device, &command_pool_create_info, &vk_alloc, &command_pool));
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

    rx::vector<VkDescriptorSetLayout> VulkanRenderDevice::create_descriptor_set_layouts(
        const rx::map<rx::string, ResourceBindingDescription>& all_bindings,
        rx::vector<uint32_t>& variable_descriptor_counts,
        rx::memory::allocator* allocator) const {

        /*
         * A few tasks to accomplish:
         * - Take the unordered map of descriptor sets (all_bindings) and convert it into
         *      VkDescriptorSetLayoutCreateInfo structs, ordering everything along the way
         * -
         */

        uint32_t num_sets = 0;
        all_bindings.each_value([&](const ResourceBindingDescription& desc) {
            if(desc.set >= gpu.props.limits.maxBoundDescriptorSets) {
                logger(rx::log::level::k_error,
                       "Descriptor set %u is out of range - your GPU only supports %u sets!",
                       desc.set,
                       gpu.props.limits.maxBoundDescriptorSets);
            } else {
                num_sets = rx::algorithm::max(num_sets, desc.set + 1);
            }
        });

        variable_descriptor_counts.resize(num_sets, 0);

        // Some precalculations so we know how much room we actually need
        rx::vector<uint32_t> num_bindings_per_set{allocator};
        num_bindings_per_set.resize(num_sets);

        all_bindings.each_value([&](const ResourceBindingDescription& desc) {
            num_bindings_per_set[desc.set] = rx::algorithm::max(num_bindings_per_set[desc.set], desc.binding + 1);
        });

        rx::vector<rx::vector<VkDescriptorSetLayoutBinding>> bindings_by_set{allocator};
        rx::vector<rx::vector<VkDescriptorBindingFlags>> binding_flags_by_set{allocator};
        bindings_by_set.reserve(num_sets);
        binding_flags_by_set.reserve(num_sets);

        uint32_t set = 0;

        num_bindings_per_set.each_fwd([&](const uint32_t num_bindings) {
            // Emplace back vectors large enough to hold all the bindings we have
            bindings_by_set.emplace_back(num_bindings);
            binding_flags_by_set.emplace_back(num_bindings);

            logger(rx::log::level::k_verbose, "Set %u has %u bindings", set, num_bindings);
            set++;
        });

        all_bindings.each_value([&](const ResourceBindingDescription& binding) {
            if(binding.set >= bindings_by_set.size()) {
                logger(rx::log::level::k_error, "You've skipped one or more descriptor sets! Don't do that, Nova can't handle it");
                return true;
            }

            VkDescriptorSetLayoutBinding descriptor_binding = {};
            descriptor_binding.binding = binding.binding;
            descriptor_binding.descriptorType = to_vk_descriptor_type(binding.type);
            descriptor_binding.descriptorCount = binding.count;
            descriptor_binding.stageFlags = to_vk_shader_stage_flags(binding.stages);

            logger(rx::log::level::k_verbose,
                   "Descriptor %u.%u is type %s",
                   binding.set,
                   binding.binding,
                   descriptor_type_to_string(binding.type));

            if(binding.is_unbounded) {
                binding_flags_by_set[binding.set][binding.binding] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                                                                     VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

                // Record the maximum number of descriptors in the variable size array in this set
                variable_descriptor_counts[binding.set] = binding.count;

                logger(rx::log::level::k_verbose, "Descriptor %u.%u is unbounded", binding.set, binding.binding);

            } else {
                binding_flags_by_set[binding.set][binding.binding] = 0;
            }

            bindings_by_set[binding.set][binding.binding] = descriptor_binding;

            return true;
        });

        rx::vector<VkDescriptorSetLayoutCreateInfo> dsl_create_infos{allocator};
        dsl_create_infos.reserve(bindings_by_set.size());

        rx::vector<VkDescriptorSetLayoutBindingFlagsCreateInfo> flag_infos{allocator};
        flag_infos.reserve(bindings_by_set.size());

        // We may make bindings_by_set much larger than it needs to be is there's multiple descriptor bindings per set. Thus, only iterate
        // through the sets we actually care about
        bindings_by_set.each_fwd([&](const rx::vector<VkDescriptorSetLayoutBinding>& bindings) {
            VkDescriptorSetLayoutCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            create_info.bindingCount = static_cast<uint32_t>(bindings.size());
            create_info.pBindings = bindings.data();

            const auto& flags = binding_flags_by_set[dsl_create_infos.size()];
            VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags = {};
            binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
            binding_flags.bindingCount = static_cast<uint32_t>(flags.size());
            binding_flags.pBindingFlags = flags.data();
            flag_infos.emplace_back(binding_flags);

            create_info.pNext = &flag_infos[flag_infos.size() - 1];

            dsl_create_infos.push_back(create_info);
        });

        rx::vector<VkDescriptorSetLayout> layouts{allocator};
        auto vk_alloc = wrap_allocator(allocator);
        layouts.resize(dsl_create_infos.size());
        for(size_t i = 0; i < dsl_create_infos.size(); i++) {
            vkCreateDescriptorSetLayout(device, &dsl_create_infos[i], &vk_alloc, &layouts[i]);
        }

        return layouts;
    }

    VkImageView VulkanRenderDevice::image_view_for_image(const Image* image) {
        // TODO: This method is terrible. We shouldn't tie image views to images, we should let everything that wants
        // to use the image create its own image view

        const auto* vk_image = static_cast<const VulkanImage*>(image);

        return vk_image->image_view;
    }

    VkCommandBufferLevel VulkanRenderDevice::to_vk_command_buffer_level(const CommandList::Level level) {
        switch(level) {
            case CommandList::Level::Primary:
                return VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            case CommandList::Level::Secondary:
                return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        }

        return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    }

    VulkanInputAssemblerLayout VulkanRenderDevice::get_input_assembler_setup(const rx::vector<VertexField>& vertex_fields) {
        rx::vector<VkVertexInputAttributeDescription> attributes;
        rx::vector<VkVertexInputBindingDescription> bindings;

        attributes.reserve(vertex_fields.size());
        bindings.reserve(vertex_fields.size());

        uint32_t cur_binding = 0;
        uint32_t byte_offset = 0;
        vertex_fields.each_fwd([&](const VertexField& field) {
            const auto attr_format = to_vk_vertex_format(field.format);
            attributes.emplace_back(VkVertexInputAttributeDescription{cur_binding, 0, attr_format, byte_offset});

            bindings.emplace_back(VkVertexInputBindingDescription{cur_binding, sizeof(FullVertex), VK_VERTEX_INPUT_RATE_VERTEX});

            cur_binding++;
            byte_offset += get_byte_size(field.format);
        });

        return {attributes, bindings};
    }

    rx::optional<VkShaderModule> VulkanRenderDevice::create_shader_module(const rx::vector<uint32_t>& spirv) const {
        VkShaderModuleCreateInfo shader_module_create_info = {};
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.pCode = spirv.data();
        shader_module_create_info.codeSize = spirv.size() * 4;

        VkShaderModule module;
        auto vk_alloc = wrap_allocator(internal_allocator);
        const auto result = vkCreateShaderModule(device, &shader_module_create_info, &vk_alloc, &module);
        if(result == VK_SUCCESS) {
            return rx::optional<VkShaderModule>(module);

        } else {
            logger(rx::log::level::k_error, "Could not create shader module: %s", to_string(result));
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
            logger(rx::log::level::k_error, "%s", msg);
#ifdef NOVA_LINUX
            nova_backtrace();
#endif

            auto* vk_render_device = reinterpret_cast<VulkanRenderDevice*>(render_device);
            if(vk_render_device->settings->debug.break_on_validation_errors) {
#if defined(NOVA_WINDOWS)
                DebugBreak();
#elif defined(NOVA_LINUX)
                raise(SIGINT);
#endif
            }

        } else if((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
            // Warnings may hint at unexpected / non-spec API usage
            logger(rx::log::level::k_warning, "%s", msg);

        } else if(((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) &&
                  ((message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) == 0U)) { // No validation info!
            // Informal messages that may become handy during debugging
            logger(rx::log::level::k_info, "%s", msg);

        } else if((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) != 0) {
            // Diagnostic info from the Vulkan loader and layers
            // Usually not helpful in terms of API usage, but may help to debug layer and loader problems
            logger(rx::log::level::k_verbose, "%s", msg);

        } else if((message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) == 0U) { // No validation info!
            // Catch-all to be super sure
            logger(rx::log::level::k_info, "%s", msg);
        }

        return VK_FALSE;
    }
} // namespace nova::renderer::rhi
