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
        vk::DeviceMemory memory;
    };

    struct VulkanSampler : RhiSampler {
        vk::Sampler sampler;
    };

    struct VulkanImage : RhiImage {
        vk::Image image = VK_NULL_HANDLE;
        vk::ImageView image_view = VK_NULL_HANDLE;
        VmaAllocation allocation{};
    };

    struct VulkanBuffer : RhiBuffer {
        vk::Buffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation{};
        VmaAllocationInfo allocation_info{};
    };

    struct VulkanMaterialResources : RhiMaterialResources {
        vk::DescriptorSet set;
    };

    struct VulkanPipelineLayoutInfo {
        std::unordered_map<std::string, RhiResourceBindingDescription> bindings;

        std::vector<vk::DescriptorSetLayout> descriptor_set_layouts;

        vk::PipelineLayout layout;

        std::vector<uint32_t> variable_descriptor_set_counts;
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
        vk::RenderPass pass = VK_NULL_HANDLE;
        vk::Rect2D render_area{};

        /*!
         * \brief Cache of pipelines that get used in this renderpass
         *
         * We keep a cache of PSOs that are used by this renderpass, using the frontend name of the pipeline state as a key. If we've
         * already used a pipeline state with this renderpass we just get the caches PSO, otherwise we have to create it
         */
        std::unordered_map<std::string, vk::Pipeline> cached_pipelines;
    };

    struct VulkanFramebuffer : RhiFramebuffer {
        vk::Framebuffer framebuffer = VK_NULL_HANDLE;
    };

    struct VulkanPipelineInterface : RhiPipelineInterface {
        /*!
         * \brief Renderpass for the pipeline's output layouts because why _wouldn't_ that be married to the
         * renderpass itself?
         */
        vk::RenderPass pass = VK_NULL_HANDLE;
    };

    struct VulkanDescriptorPool : RhiDescriptorPool {
        vk::DescriptorPool descriptor_pool{};
    };

    struct VulkanDescriptorSet : RhiDescriptorSet {
        vk::DescriptorSet descriptor_set;
    };

    struct VulkanSemaphore : RhiSemaphore {
        vk::Semaphore semaphore;
    };

    struct VulkanFence : RhiFence {
        vk::Fence fence;
    };

    struct VulkanGpuInfo {
        vk::PhysicalDevice phys_device{};
        std::vector<vk::QueueFamilyProperties> queue_family_props;
        std::vector<vk::ExtensionProperties> available_extensions;
        vk::SurfaceCapabilitiesKHR surface_capabilities{};
        std::vector<vk::SurfaceFormatKHR> surface_formats;
        vk::PhysicalDeviceProperties props{};
        vk::PhysicalDeviceFeatures supported_features{};
        vk::PhysicalDeviceMemoryProperties memory_properties{};
    };
} // namespace nova::renderer::rhi
