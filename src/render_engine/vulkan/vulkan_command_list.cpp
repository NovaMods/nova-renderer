/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#include "vulkan_command_list.hpp"

namespace nova::renderer {
    vulkan_command_list::vulkan_command_list(VkCommandBuffer cmds) : cmds(cmds) {}

    void vulkan_command_list::resource_barrier(const std::vector<resource_barrier_t>& barriers) {
        std::vector<VkMemoryBarrier> memory_barriers;
        memory_barriers.reserve(barriers.size());

        std::vector<VkBufferMemoryBarrier> buffer_barriers;
        buffer_barriers.reserve(barriers.size());

        std::vector<VkImageMemoryBarrier> image_barriers;
        image_barriers.reserve(barriers.size());
    }
} // namespace nova::renderer
