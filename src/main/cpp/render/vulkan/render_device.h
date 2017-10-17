/*!
 * \author ddubois 
 * \date 15-Oct-17.
 */

#ifndef RENDERER_RENDER_DEVICE_H
#define RENDERER_RENDER_DEVICE_H

#include <vulkan/vulkan.h>
#include "../windowing/glfw_vk_window.h"

namespace nova {
#define NUM_FRAME_DATA 2    /* I guess? */

    struct gpu_info {
        VkPhysicalDevice device;
        std::vector<VkQueueFamilyProperties> queue_family_props;
        std::vector<VkExtensionProperties> extention_props;
        VkSurfaceCapabilitiesKHR surface_capabilities;
        std::vector<VkSurfaceFormatKHR> surface_formats;
        VkPhysicalDeviceMemoryProperties mem_props;
        VkPhysicalDeviceProperties props;
        VkPhysicalDeviceFeatures supported_features;
        std::vector<VkPresentModeKHR> present_modes;
    };

    /*!
     * \brief An abstraction over Vulkan physical and logical devices
     *
     * Pretty sure I need one command buffer pool per thread that does things
     */
    class render_device {
    public:
        VkInstance instance = nullptr;
        VkSurfaceKHR surface;

        VkDebugReportCallbackEXT callback;

        uint32_t graphics_family_idx;
        uint32_t present_family_idx;
        gpu_info *gpu;

        VkPhysicalDevice physical_device;
        VkDevice device;

        VkQueue graphics_queue;
        VkQueue present_queue;

        void create_instance(glfw_vk_window &window);

        void setup_debug_callback();

        void find_device_and_queues();

        void create_semaphores();

        void create_command_pool_and_command_buffers();

    private:
        std::vector<gpu_info> gpus;

        std::vector<const char *> validation_layers;
        std::vector<const char *> extensions;

        std::vector<VkSemaphore> acquire_semaphores;
        std::vector<VkSemaphore> render_complete_semaphores;

        void create_logical_device_and_queues();

        void enumerate_gpus();

        void select_physical_device();

        void create_command_pool();
    };

    VkResult
    CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback);

    void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                       const VkAllocationCallbacks *pAllocator);
}

#endif //RENDERER_RENDER_DEVICE_H
