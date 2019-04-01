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

        for(const resource_barrier_t& barrier : barriers) {
            switch(barrier.resource_to_barrier->type) {
                case resource_t::type::IMAGE:
                    VkImageMemoryBarrier image_barrier = {};
                    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    image_barrier.image = barrier.resource_to_barrier->image;

                case resource_t::type::BUFFER:
                    break;
            }
        }
    }

    void vulkan_command_list::copy_buffer(resource_t* destination_buffer,
                                          const uint64_t destination_offset,
                                          resource_t* source_buffer,
                                          const uint64_t source_offset,
                                          const uint64_t num_bytes) {
        VkBufferCopy copy = {};
        copy.srcOffset = source_offset;
        copy.dstOffset = destination_offset;
        copy.size = num_bytes;

        vkCmdCopyBuffer(cmds, source_buffer->buffer, destination_buffer->buffer, 1, &copy);
    }
} // namespace nova::renderer
