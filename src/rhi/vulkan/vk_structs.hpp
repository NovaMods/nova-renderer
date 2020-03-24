/*!
 * \brief Vulkan definition of the structs forward-declared in render_device.hpp
 */

#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "nova_renderer/rhi/pipeline_create_info.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"

namespace nova::renderer::rhi {
    struct VulkanDeviceMemory : RhiDeviceMemory {
        VkDeviceMemory memory;
    };

    struct VulkanSampler : RhiSampler {
        VkSampler sampler;
    };

    struct VulkanImage : RhiImage {
        VkImage image = VK_NULL_HANDLE;
        VkImageView image_view = VK_NULL_HANDLE;
        VmaAllocation allocation{};
    };

    struct VulkanBuffer : RhiBuffer {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation{};
        VmaAllocationInfo allocation_info{};
    };

    struct VulkanMaterialResources : RhiMaterialResources {
        vk::DescriptorSet set;
    };

    struct VulkanPipelineLayoutInfo {
        rx::map<rx::string, RhiResourceBindingDescription> bindings;

        rx::vector<vk::DescriptorSetLayout> descriptor_set_layouts;

        vk::PipelineLayout layout;
    };

    /*!
     * \brief Represents a Vulkan pipeline
     *
     * Vulkan pipelines are actually compiled lazily, because they depend on the layouts of the render targets they
     * write to. This struct just contains the input layout of the pipeline and the PSO create info, which we combine
     * with a renderpass to compile the pipeline
     */
    struct VulkanPipeline : RhiPipeline {
        RhiGraphicsPipelineState state;

        VulkanPipelineLayoutInfo layout;
    };

    struct VulkanRenderpass : RhiRenderpass {
        VkRenderPass pass = VK_NULL_HANDLE;
        VkRect2D render_area{};

        /*!
         * \brief Cache of pipelines that get used in this renderpass
         *
         * We keep a cache of PSOs that are used by this renderpass, using the frontend name of the pipeline state as a key. If we've
         * already used a pipeline state with this renderpass we just get the caches PSO, otherwise we have to create it
         */
        rx::map<rx::string, vk::Pipeline> cached_pipelines;
    };

    struct VulkanFramebuffer : RhiFramebuffer {
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
    };

    struct VulkanPipelineInterface : RhiPipelineInterface {
        /*!
         * \brief Renderpass for the pipeline's output layouts because why _wouldn't_ that be married to the
         * renderpass itself?
         */
        VkRenderPass pass = VK_NULL_HANDLE;
    };

    struct VulkanDescriptorPool : RhiDescriptorPool {
        vk::DescriptorPool descriptor_pool{};
    };

    struct VulkanDescriptorSet : RhiDescriptorSet {
        VkDescriptorSet descriptor_set;
    };

    struct VulkanSemaphore : RhiSemaphore {
        VkSemaphore semaphore;
    };

    struct VulkanFence : RhiFence {
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
