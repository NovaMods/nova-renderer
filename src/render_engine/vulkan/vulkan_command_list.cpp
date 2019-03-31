/*!
 * \author ddubois 
 * \date 30-Mar-19.
 */

#include "vulkan_command_list.hpp"

namespace nova::renderer {
    vulkan_command_list::vulkan_command_list(VkCommandBuffer cmds) : cmds(cmds) {}
}