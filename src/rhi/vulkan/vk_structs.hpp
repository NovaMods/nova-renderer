/*!
 * \brief Vulkan definition of the structs forward-declared in render_device.hpp
 */

#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "nova_renderer/rhi/pipeline_create_info.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"

namespace nova::renderer::rhi {
    struct VulkanSampler final : RhiSampler {
        VkSampler sampler;

        ~VulkanSampler() override = default;
    };

    struct VulkanImage final : RhiImage {
        VkImage image = VK_NULL_HANDLE;
        VkImageView image_view = VK_NULL_HANDLE;
        VmaAllocation allocation{};

        ~VulkanImage() override = default;
    };

    struct VulkanBuffer final : RhiBuffer {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation{};
        VmaAllocationInfo allocation_info{};

        ~VulkanBuffer() override = default;
    };

    struct VulkanPipelineLayoutInfo {
        rx::map<rx::string, RhiResourceBindingDescription> bindings;

        rx::vector<vk::DescriptorSetLayout> descriptor_set_layouts;

        vk::PipelineLayout layout;

        rx::vector<uint32_t> variable_descriptor_set_counts;
    };

    /*!
     * \brief Represents a Vulkan pipeline
     *
     * Vulkan pipelines are actually compiled lazily, because they depend on the layouts of the render targets they
     * write to. This struct just contains the input layout of the pipeline and the PSO create info, which we combine
     * with a renderpass to compile the pipeline
     */
    struct VulkanPipeline final : RhiPipeline {
        RhiGraphicsPipelineState state;

        VulkanPipelineLayoutInfo layout;

        ~VulkanPipeline() override = default;
    };

    struct VulkanRenderpass final : RhiRenderpass {
        VkRenderPass pass = VK_NULL_HANDLE;
        VkRect2D render_area{};

        /*!
         * \brief Cache of pipelines that get used in this renderpass
         *
         * We keep a cache of PSOs that are used by this renderpass, using the frontend name of the pipeline state as a key. If we've
         * already used a pipeline state with this renderpass we just get the caches PSO, otherwise we have to create it
         */
        rx::map<rx::string, vk::Pipeline> cached_pipelines;

        ~VulkanRenderpass() override = default;
    };

    struct VulkanFramebuffer final : RhiFramebuffer {
        VkFramebuffer framebuffer = VK_NULL_HANDLE;

        ~VulkanFramebuffer() override = default;
    };

    struct VulkanSemaphore final : RhiSemaphore {
        VkSemaphore semaphore;

        ~VulkanSemaphore() override = default;
    };

    struct VulkanFence final : RhiFence {
        ~VulkanFence() override = default;

        VkFence fence;
    };

    struct VulkanGpuInfo {
        VkPhysicalDevice phys_device{};
        rx::vector<VkQueueFamilyProperties> queue_family_props;
        rx::vector<VkExtensionProperties> available_extensions;
        VkSurfaceCapabilitiesKHR surface_capabilities{};
        rx::vector<VkSurfaceFormatKHR> surface_formats;
        VkPhysicalDeviceProperties props{};
        VkPhysicalDeviceFeatures supported_features{};
        VkPhysicalDeviceMemoryProperties memory_properties{};
    };
} // namespace nova::renderer::rhi
