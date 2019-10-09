/*!
 * \brief Vulkan definition of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois
 * \date 01-Apr-19.
 */

#pragma once

#include <vulkan/vulkan.hpp>

#include "nova_renderer/rhi_types.hpp"

namespace nova::renderer::rhi {
    struct VulkanDeviceMemory : DeviceMemory {
        VkDeviceMemory memory;
    };

    struct VulkanSampler : Sampler {
        VkSampler sampler;
    };

    struct VulkanImage : Image {
        VkImage image = nullptr;
        VkImageView image_view = nullptr;
        VulkanDeviceMemory* memory = nullptr;
    };

    struct VulkanBuffer : Buffer {
        VkBuffer buffer = nullptr;
        DeviceMemoryAllocation memory{};
    };

    struct VulkanRenderpass : Renderpass {
        VkRenderPass pass = nullptr;
        VkRect2D render_area{};
    };

    struct VulkanFramebuffer : Framebuffer {
        VkFramebuffer framebuffer = nullptr;
    };

    struct VulkanPipelineInterface : PipelineInterface {
        /*!
         * \brief Renderpass for the pipeline's output layouts because why _wouldn't_ that be married to the
         * renderpass itself?
         */
        VkRenderPass pass = nullptr;

        VkPipelineLayout pipeline_layout = nullptr;

        /*!
         * \brief All the descriptor set layouts that this pipeline interface needs to create descriptor sets
         *
         * The index in the vector is the index of the set
         */
        std::vector<VkDescriptorSetLayout> layouts_by_set;
    };

    struct VulkanPipeline : Pipeline {
        VkPipeline pipeline = nullptr;
    };

    struct VulkanDescriptorPool : DescriptorPool {
        VkDescriptorPool descriptor_pool = nullptr;
    };

    struct VulkanDescriptorSet : DescriptorSet {
        VkDescriptorSet descriptor_set;
    };

    struct VulkanSemaphore : Semaphore {
        VkSemaphore semaphore;
    };

    struct VulkanFence : Fence {
        VkFence fence;
    };

    struct VulkanGpuInfo {
        VkPhysicalDevice phys_device{};
        std::vector<VkQueueFamilyProperties> queue_family_props;
        std::vector<VkExtensionProperties> available_extensions;
        VkSurfaceCapabilitiesKHR surface_capabilities{};
        std::vector<VkSurfaceFormatKHR> surface_formats;
        VkPhysicalDeviceProperties props{};
        VkPhysicalDeviceFeatures supported_features{};
        VkPhysicalDeviceMemoryProperties memory_properties{};
    };
} // namespace nova::renderer::rhi
