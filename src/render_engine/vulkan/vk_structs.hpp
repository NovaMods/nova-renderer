/*!
 * \brief Vulkan definition of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois
 * \date 01-Apr-19.
 */

#pragma once

//#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "nova_renderer/rhi_types.hpp"

#ifdef ENABLE_VULKAN
namespace nova::renderer::rhi {

    struct VulkanDeviceMemory : DeviceMemory {
        // vulkan API memory handler
        VkDeviceMemory memory = nullptr;
        DeviceMemoryAllocation *allocation = nullptr; // if has allocation 

        // implicit cast into VkDeviceMemory
        operator VkDeviceMemory&() { return memory; };
        operator const VkDeviceMemory&() const { return memory; };

        // may fail 
        operator DeviceMemoryAllocation&() { return *allocation; };
        operator const DeviceMemoryAllocation&() const { return *allocation; };
    };

    struct VulkanSampler : Sampler {
        VkSampler sampler = nullptr;

        operator VkSampler&() { return sampler; };
        operator const VkSampler&() const { return sampler; };
    };

    struct VulkanImage : Image {
        VkImage image = nullptr;
        VkImageView image_view = nullptr;
        VulkanDeviceMemory* memory = nullptr;
        // also should to be have image info for manipulations with image

        operator VulkanDeviceMemory&() { return *memory; };
        operator const VulkanDeviceMemory&() const { return *memory; };
        operator VkImage&() { return image; };
        operator const VkImage&() const { return image; };
        operator VkImageView&() { return image_view; };
        operator const VkImageView&() const { return image_view; };
    };

    struct VulkanBuffer : Buffer {
        VkBuffer buffer = nullptr; VkDeviceSize offset = 0u, range = 0u;
        VulkanDeviceMemory* memory = nullptr;
        // also better to have range info and descriptor write buffer into (region)

        virtual uint64_t& size() override final { return range; };
        virtual const uint64_t& size() const override final { return range; };

        operator VulkanDeviceMemory&() { return *memory; };
        operator const VulkanDeviceMemory&() const { return *memory; };
        operator VkBuffer&() { return buffer; };
        operator const VkBuffer&() const { return buffer; };
    };

    struct VulkanRenderpass : Renderpass {
        VkRenderPass pass = nullptr;
        VkRect2D render_area{VkOffset2D{0,0},VkExtent2D{0,0}};

        operator VkRenderPass&() { return pass; };
        operator const VkRenderPass&() const { return pass; };
    };

    struct VulkanFramebuffer : Framebuffer {
        VkFramebuffer framebuffer = nullptr;

        operator VkFramebuffer&() { return framebuffer; };
        operator const VkFramebuffer&() const { return framebuffer; };
    };

    struct VulkanPipelineInterface : PipelineInterface {
        /*!
         * \brief Renderpass for the pipeline's output layouts because why _wouldn't_ that be married to the
         * renderpass itself?
         */
        VkRenderPass pass = nullptr;
        VkPipelineLayout pipeline_layout = nullptr;

        operator VkRenderPass&() { return pass; };
        operator const VkRenderPass&() const { return pass; };
        operator VkPipelineLayout&() { return pipeline_layout; };
        operator const VkPipelineLayout&() const { return pipeline_layout; };

        /*!
         * \brief All the descriptor set layouts that this pipeline interface needs to create descriptor sets
         *
         * The index in the vector is the index of the set
         */
        std::vector<VkDescriptorSetLayout> layouts_by_set{};
        
        operator std::vector<VkDescriptorSetLayout>&() { return layouts_by_set; };
        operator const std::vector<VkDescriptorSetLayout>&() const { return layouts_by_set; };
    };

    struct VulkanPipeline : Pipeline {
        VkPipeline pipeline = nullptr;
        VulkanPipelineInterface* pipeline_interface = nullptr; // added for simpler binding

        operator VkPipeline&() { return pipeline; };
        operator const VkPipeline&() const { return pipeline; };
        operator VulkanPipelineInterface&() { return *pipeline_interface; };
        operator const VulkanPipelineInterface&() const { return *pipeline_interface; };
    };

    struct VulkanDescriptorPool : DescriptorPool {
        VkDescriptorPool descriptor_pool = nullptr;

        operator VkDescriptorPool&() { return descriptor_pool; };
        operator const VkDescriptorPool&() const { return descriptor_pool; };
    };

    struct VulkanDescriptorSet : DescriptorSet {
        VkDescriptorSet descriptor_set = nullptr;

        operator VkDescriptorSet&() { return descriptor_set; };
        operator const VkDescriptorSet&() const { return descriptor_set; };
    };

    struct VulkanSemaphore : Semaphore {
        VkSemaphore semaphore = nullptr;

        operator VkSemaphore&() { return semaphore; };
        operator const VkSemaphore&() const { return semaphore; };
    };

    struct VulkanFence : Fence { // should to be point into CPU thread/instance?
        VkFence fence = nullptr;

        operator VkFence&() { return fence; };
        operator const VkFence&() const { return fence; };
    };

    struct VulkanGpuInfo {
        VkPhysicalDevice phys_device = nullptr;
        VkPhysicalDeviceProperties2 properties{};
        VkPhysicalDeviceFeatures2 features{};
        VkPhysicalDeviceMemoryProperties2 memory_properties{};
        VkSurfaceCapabilitiesKHR surface_capabilities{};
        std::vector<VkExtensionProperties> available_extensions;
        std::vector<VkQueueFamilyProperties> queue_family_props;
        std::vector<VkSurfaceFormatKHR> surface_formats;
    };
} // namespace nova::renderer::rhi
#endif
