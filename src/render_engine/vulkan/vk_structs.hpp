/*!
 * \brief Vulkan definition of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois
 * \date 01-Apr-19.
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

#include "nova_renderer/rhi_types.hpp"

namespace nova::renderer::rhi {
    struct VulkanImage : Image {
        VkImage image;
        VkImageView image_view;
        VmaAllocation allocation;
        VmaAllocationInfo vma_info{};
    };

    struct VulkanBuffer : Buffer {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo vma_info{};
    };

    struct VulkanRenderpass : Renderpass {
        VkRenderPass pass;
        VkRect2D render_area{};
    };

    struct VulkanFramebuffer : Framebuffer {
        VkFramebuffer framebuffer;
    };

    struct vk_pipeline_t : Pipeline {
        VkPipeline pipeline;
    };

    struct vk_gpu_info {
        VkPhysicalDevice phys_device{};
        std::vector<VkQueueFamilyProperties> queue_family_props;
        std::vector<VkExtensionProperties> available_extensions;
        VkSurfaceCapabilitiesKHR surface_capabilities{};
        std::vector<VkSurfaceFormatKHR> surface_formats;
        VkPhysicalDeviceProperties props{};
        VkPhysicalDeviceFeatures supported_features{};
    };
} // namespace nova::renderer::rhi
