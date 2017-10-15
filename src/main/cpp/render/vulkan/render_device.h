/*!
 * \author ddubois 
 * \date 15-Oct-17.
 */

#ifndef RENDERER_RENDER_DEVICE_H
#define RENDERER_RENDER_DEVICE_H

#include <vulkan/vulkan.h>

/*!
 * \brief An abstraction over Vulkan physical and logical devices
 */
class render_device {
public:
    explicit render_device(VkInstance instance);
};


#endif //RENDERER_RENDER_DEVICE_H
