/*!
 * \author ddubois 
 * \date 15-Oct-17.
 */

#define NUM_FRAME_DATA 2    /* I guess? */

#ifndef RENDERER_RENDER_DEVICE_H
#define RENDERER_RENDER_DEVICE_H

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>

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
    class render_context {
    public:
        vk::Instance vk_instance = nullptr;
        vk::SurfaceKHR surface;

        vk::DebugReportCallbackEXT callback;

        uint32_t graphics_family_idx;
        uint32_t present_family_idx;
        gpu_info gpu;

        vk::PhysicalDevice physical_device;
        vk::Device device;

        vk::Queue graphics_queue;
        vk::Queue present_queue;
        std::unique_ptr<command_pool> command_buffer_pool;

        std::vector<gpu_info> gpus;

        std::vector<const char *> validation_layers;
        std::vector<const char *> extensions;

        std::vector<vk::Semaphore> acquire_semaphores;
        std::vector<vk::Semaphore> render_complete_semaphores;

        VmaAllocator allocator;

        vk::PipelineCache pipeline_cache;

        ~render_context();

        void create_instance(glfw_vk_window &window);

        void setup_debug_callback();

        void find_device_and_queues();

        void create_semaphores();

        void create_command_pool_and_command_buffers();

        void create_pipeline_cache();

    private:

        void create_logical_device_and_queues();

        void enumerate_gpus();

        void select_physical_device();

        PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
        PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;
        VkDebugReportCallbackEXT debug_report_callback;
    };
}

#endif //RENDERER_RENDER_DEVICE_H
