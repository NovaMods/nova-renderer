/*!
 * \author ddubois 
 * \date 15-Oct-17.
 */

#ifndef RENDERER_RENDER_DEVICE_H
#define RENDERER_RENDER_DEVICE_H

#include <vulkan/vulkan.h>

struct gpu_info {
    VkPhysicalDevice device;
};

/*!
 * \brief An abstraction over Vulkan physical and logical devices
 */
class render_device {
public:
    VkInstance instance;
    VkSurfaceKHR surface;
    std::vector<gpu_info> gpus;

    VkDebugReportCallbackEXT debug_callback;

    void find_device_and_queues();
};


#endif //RENDERER_RENDER_DEVICE_H
