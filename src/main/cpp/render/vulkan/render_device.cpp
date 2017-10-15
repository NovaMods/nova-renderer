/*!
 * \author ddubois 
 * \date 15-Oct-17.
 */

#include <easylogging++.h>
#include "render_device.h"

void render_device::find_device_and_queues() {
    uint32_t num_devices = 0;
    auto err = vkEnumeratePhysicalDevices(instance, &num_devices, nullptr);
    if(err != VK_SUCCESS) {
        LOG(FATAL) << "Could not enumerate devices. Are you sure you have a GPU?";
    }

    if(num_devices == 0) {
        LOG(FATAL) << "Apparently you have zero devices. You know you need a GPU to run Nova, right>";
    }

    std::vector<VkPhysicalDevice> devices(num_devices);
    err = vkEnumeratePhysicalDevices(instance, &num_devices, devices.data());
    if(err != VK_SUCCESS) {
        LOG(FATAL) << "Could not enumerate physical devices";
    }


    for(uint32_t i = 0; i < num_devices; i++) {
        gpu_info gpu;
    }
}
