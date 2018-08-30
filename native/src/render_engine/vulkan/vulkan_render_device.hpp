/*!
 * \author ddubois 
 * \date 14-Aug-18.
 */

#ifndef NOVA_RENDERER_VULKAN_RENDER_DEVICE_H
#define NOVA_RENDERER_VULKAN_RENDER_DEVICE_H

#include "../../platform.hpp"

#ifdef SUPPORT_VULKAN

#include <vulkan/vulkan.hpp>
#include <cstdint>
#include <vk_mem_alloc.h>

namespace nova {
    class glfw_vk_window;
    class command_pool;
    class timestamp_query_pool;

    struct gpu_info {
        vk::PhysicalDevice device;
        std::vector<vk::QueueFamilyProperties> queue_family_props;
        std::vector<vk::ExtensionProperties> extention_props;
        vk::SurfaceCapabilitiesKHR surface_capabilities;
        std::vector<vk::SurfaceFormatKHR> surface_formats;
        vk::PhysicalDeviceMemoryProperties mem_props;
        vk::PhysicalDeviceProperties props;
        vk::PhysicalDeviceFeatures supported_features;
        std::vector<vk::PresentModeKHR> present_modes;
    };

    enum class queue_type {
        GRAPHICS,
        TRANSFER,
        PRESENT,
        ASYNC_COMPUTE,
    };

    /*!
     * \brief Holds global render-related resources
     *
     * Some example resources:
     *  - VkDevice and VkInstance
     *  - Information about the GPU we're using
     *  - Information about timestamp queries
     *  - All the queues that Nova uses
     */
    class vulkan_render_context {
    public:
        vk::Instance vk_instance = nullptr;
        vk::SurfaceKHR surface;
        gpu_info gpu;

        std::vector<vk::Semaphore> acquire_semaphores;
        std::vector<vk::Semaphore> render_complete_semaphores;

        vk::PipelineCache pipeline_cache;

        /*!
         * \brief Creates a Vulkan render device. Initializes the Vulkan context, selects a GPU to use, created the
         * queues we'll eventually use to submit everything, etc
         *
         * \param window The window we're rendering to
         */
        explicit vulkan_render_context(glfw_vk_window& window);

        /*!
         * \brief Destructs this render device, cleaning up stuff
         *
         * This destructor should be one of the last things to run. I'm not convinced we actually need to clean up
         * anything - the validation layers will complain, sure, but the OS should clean up all our memory and tell the
         * graphics driver to clean up any memory from Nova
         */
        ~vulkan_render_context();
        
        /*!
         * \brief Submits one or more command buffers to the specified queue
         * \param submit_info The submission info for the command buffers you want to submit
         * \param submission_queue The type of queue you want to use. Currently Nova only supports one queue of each type
         * \param completion_fence An optional fence to signal when the command buffer is done if you want to e.g. clean
         * up some resources that a command buffer was using
         */
        void submit_command_buffer(const vk::SubmitInfo& submit_info, const queue_type& submission_queue, const vk::Fence& completion_fence = vk::Fence());

        VmaAllocator& get_device_allocator() const;

        vk::Device& get_vk_device() const;

    private:
        void create_instance(glfw_vk_window &window);

        void setup_debug_callback();

        void find_device_and_queues();

        void create_pipeline_cache();

        void create_logical_device_and_queues(const gpu_info& info);

        std::vector<gpu_info> enumerate_gpus();

        const gpu_info select_physical_device(const std::vector<gpu_info>& vector);

        // Needed in both create_instance and create_logical_device_and_queues
        std::vector<const char *> validation_layers;

        uint32_t graphics_family_idx;
        uint32_t present_family_idx;

        vk::Queue graphics_queue;
        vk::Queue present_queue;

        vk::PhysicalDevice physical_device;

        vk::DebugReportCallbackEXT callback;

        VmaAllocator allocator;

        PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
        PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;
        VkDebugReportCallbackEXT debug_report_callback;

        vk::Device device;

        std::unique_ptr<timestamp_query_pool> timestamp_queries;
    };
}

#endif

#endif //NOVA_RENDERER_VULKAN_RENDER_DEVICE_H
