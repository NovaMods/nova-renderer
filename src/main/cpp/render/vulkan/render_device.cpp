/*!
 * \author ddubois 
 * \date 15-Oct-17.
 */

#include <easylogging++.h>
#include "render_device.h"

render_device::render_device(VkInstance instance) {
    uint32_t num_devices = 0;
    auto err = vkEnumeratePhysicalDevices(instance, &num_devices, nullptr);
    if(err != VK_SUCCESS) {
        LOG(FATAL) << "Could not enumerate devices. Are you sure you have a GPU?";
    }

    if(num_devices == 0) {
        LOG(FATAL) << "Apparently you have zero devices. You know you need a GPU to run Nova, right>";
    }

    std::vector<VkPhysicalDevice> devices(num_devices);

    for(uint32_t i = 0; i < num_devices; i++) {

    }
}
