/*!
 * \author ddubois 
 * \date 15-Oct-17.
 */

#ifndef RENDERER_VULKAN_INSTANCE_H
#define RENDERER_VULKAN_INSTANCE_H

#include <vulkan/vulkan.h>
#include "windowing/glfw_vk_window.h"

namespace nova {
    VkInstance create_instance(glfw_vk_window& window);
}


#endif //RENDERER_VULKAN_INSTANCE_H
