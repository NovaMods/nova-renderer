/*!
 * \brief Vulkan definition of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois
 * \date 01-Apr-19.
 */

#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "rhi_types.hpp"
#include "radx/radx.hpp"

namespace nova::renderer::rhi {

    struct VulkanSampler : Sampler {
        vk::Sampler sampler{};
        operator vk::Sampler&() { return sampler; };
        operator const vk::Sampler&() const { return sampler; };
    };

    using VulkanImage = radx::VmaAllocatedImage;
    using VulkanBuffer = radx::VmaAllocatedBuffer;

    //struct VulkanImage : Image {
    //    vk::Image image{};
    //    vk::ImageView image_view{};
    //    std::shared_ptr<VulkanMemoryAllocation> memory{};
    //    operator vk::Image&() { return image; };
    //    operator const vk::Image&() const { return image; };
    //    operator vk::ImageView&() { return image_view; };
    //    operator const vk::ImageView&() const { return image_view; };
    //};

    //struct VulkanBuffer : Buffer {
    //    vk::Buffer buffer{};
    //    operator vk::Buffer&() { return buffer; };
    //    operator const vk::Buffer&() const { return buffer; };
    //    std::shared_ptr<VulkanMemoryAllocation> memory{};
    //};

    struct VulkanRenderpass : Renderpass {
        vk::RenderPass render_pass{};
        vk::Rect2D render_area{};
        operator vk::RenderPass&() { return render_pass; };
        operator const vk::RenderPass&() const { return render_pass; };
    };

    struct VulkanFramebuffer : Framebuffer {
        vk::Framebuffer framebuffer{};
        operator vk::Framebuffer&() { return framebuffer; };
        operator const vk::Framebuffer&() const { return framebuffer; };
    };

    struct VulkanPipelineInterface : PipelineInterface {
        /*!
         * \brief Renderpass for the pipeline's output layouts because why _wouldn't_ that be married to the
         * renderpass itself?
         */
        vk::RenderPass render_pass{};
        operator vk::RenderPass&() { return render_pass; };
        operator const vk::RenderPass&() const { return render_pass; };

        vk::PipelineLayout pipeline_layout{};
        operator vk::PipelineLayout&() { return pipeline_layout; };
        operator const vk::PipelineLayout&() const { return pipeline_layout; };

        /*!
         * \brief All the descriptor set layouts that this pipeline interface needs to create descriptor sets
         *
         * The index in the vector is the index of the set
         */
        std::vector<vk::DescriptorSetLayout> layouts_by_set{};
        operator std::vector<vk::DescriptorSetLayout>&() { return layouts_by_set; };
        operator const std::vector<vk::DescriptorSetLayout>&() const { return layouts_by_set; };
        
    };

    struct VulkanPipeline : Pipeline {
        vk::Pipeline pipeline{};
        std::shared_ptr<VulkanPipelineInterface> pipeline_interface;

        operator std::shared_ptr<VulkanPipelineInterface>&() { return pipeline_interface; };
        operator const std::shared_ptr<VulkanPipelineInterface>&() const { return pipeline_interface; };
    };

    struct VulkanDescriptorPool : DescriptorPool {
        vk::DescriptorPool descriptor_pool{};
        operator vk::DescriptorPool&() { return descriptor_pool; };
        operator const vk::DescriptorPool&() const { return descriptor_pool; };
    };

    struct VulkanDescriptorSet : DescriptorSet {
        vk::DescriptorSet descriptor_set{};
        operator vk::DescriptorSet&() { return descriptor_set; };
        operator const vk::DescriptorSet&() const { return descriptor_set; };
    };

    struct VulkanSemaphore : Semaphore {
        vk::Semaphore semaphore{};
        operator vk::Semaphore&() { return semaphore; };
        operator const vk::Semaphore&() const { return semaphore; };
    };

    struct VulkanFence : Fence {
        vk::Fence fence{};
        operator vk::Fence&() { return fence; };
        operator const vk::Fence&() const { return fence; };
    };

    struct VulkanGpuInfo {
        vk::PhysicalDevice phys_device{};
        vk::PhysicalDeviceProperties2 properties{};
        vk::PhysicalDeviceFeatures2 features{};
        vk::PhysicalDeviceMemoryProperties2 memory_properties{};
        vk::SurfaceCapabilitiesKHR surface_capabilities{};
        std::vector<vk::QueueFamilyProperties> queue_family_props{};
        std::vector<vk::ExtensionProperties> available_extensions{};
        std::vector<vk::SurfaceFormatKHR> surface_formats{};
        operator vk::PhysicalDevice&() { return phys_device; };
        operator const vk::PhysicalDevice&() const { return phys_device; };
    };
} // namespace nova::renderer::rhi
