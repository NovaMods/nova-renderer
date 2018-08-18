/*!
 * \author ddubois 
 * \date 14-Aug-18.
 */

#ifndef NOVA_RENDERER_VULKAN_RENDER_DEVICE_H
#define NOVA_RENDERER_VULKAN_RENDER_DEVICE_H

#include "../platform.h"

#ifdef SUPPORT_VULKAN

#include <vulkan/vulkan.hpp>
#include <cstdint>
#include <vk_mem_alloc.h>

namespace nova {
    class glfw_vk_window;
    class command_pool;

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
     * \brief An abstraction over Vulkan physical and logical devices
     *
     * Pretty sure I need one command buffer pool per thread that does things
     *
     * Some more things that are needed:
     *      - A texture uploading queue
     *      - A chunk uploading queue
     *      - A render queue
     *      - I've always seen things with a separate queue for presenting
     *      - One or more compute shader queues could be awesome, except that Nova won't use many compute shaders by
     *          default
     */
    class render_device {
    public:
        vk::Instance vk_instance = nullptr;
        vk::SurfaceKHR surface;
        gpu_info gpu;

        std::vector<const char *> validation_layers;
        std::vector<const char *> extensions;

        std::vector<vk::Semaphore> acquire_semaphores;
        std::vector<vk::Semaphore> render_complete_semaphores;

        vk::PipelineCache pipeline_cache;

        uint32_t timestamp_valid_bits;
        float timestamp_period;
        vk::QueryPool timestamp_query_pool;

        /*!
         * \brief Creates a Vulkan render device. Initializes the Vulkan context, selects a GPU to use, created the
         * queues we'll eventually use to submit everything, etc
         *
         * \param window The window we're rendering to
         */
        explicit render_device(glfw_vk_window& window);

        /*!
         * \brief Destructs this render device, cleaning up stuff
         *
         * This destructor should be one of the last things to run. I'm not convinced we actually need to clean up
         * anything - the validation layers will complain, sure, but the OS should clean up all our memory and tell the
         * graphics driver to clean up any memory from Nova
         */
        ~render_device();

        void recreate_timestamp_query_pool(uint32_t size);

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

        void create_semaphores();

        void create_command_pool_and_command_buffers();

        void create_pipeline_cache();

        void create_logical_device_and_queues();

        void enumerate_gpus();

        void select_physical_device();


        uint32_t graphics_family_idx;
        uint32_t present_family_idx;

        vk::Queue graphics_queue;
        vk::Queue present_queue;

        std::vector<gpu_info> gpus;

        vk::PhysicalDevice physical_device;

        vk::DebugReportCallbackEXT callback;

        VmaAllocator allocator;

        PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
        PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;
        VkDebugReportCallbackEXT debug_report_callback;

        vk::Device device;
    };
}

#endif

#endif //NOVA_RENDERER_VULKAN_RENDER_DEVICE_H
