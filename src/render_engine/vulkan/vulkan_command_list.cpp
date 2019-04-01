/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#include "vulkan_command_list.hpp"
#include "vulkan_render_engine.hpp"
#include "vulkan_utils.hpp"

namespace nova::renderer {
    vulkan_command_list::vulkan_command_list(VkCommandBuffer cmds, const vulkan_render_engine& render_engine)
        : cmds(cmds), render_engine(render_engine) {}

    void vulkan_command_list::resource_barriers(const pipeline_stage_flags stages_before_barrier,
                                               const pipeline_stage_flags stages_after_barrier,
                                               const std::vector<resource_barrier_t>& barriers) {
        std::vector<VkBufferMemoryBarrier> buffer_barriers;
        buffer_barriers.reserve(barriers.size());

        std::vector<VkImageMemoryBarrier> image_barriers;
        image_barriers.reserve(barriers.size());

        for(const resource_barrier_t& barrier : barriers) {
            switch(barrier.resource_to_barrier->resource_type) {
                case resource_t::type::IMAGE:
                    VkImageMemoryBarrier image_barrier = {};
                    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    image_barrier.srcAccessMask = barrier.access_before_barrier;
                    image_barrier.dstAccessMask = barrier.access_after_barrier;
                    image_barrier.oldLayout = to_vk_layout(barrier.initial_state);
                    image_barrier.newLayout = to_vk_layout(barrier.final_state);
                    image_barrier.srcQueueFamilyIndex = render_engine.get_queue_family_index(barrier.source_queue);
                    image_barrier.dstQueueFamilyIndex = render_engine.get_queue_family_index(barrier.destination_queue);
                    image_barrier.image = barrier.resource_to_barrier->image;
                    image_barrier.subresourceRange.aspectMask = barrier.image_memory_barrier.aspect;
                    image_barrier.subresourceRange.baseMipLevel = 0; // TODO: Something smarter with mips
                    image_barrier.subresourceRange.levelCount = 1;
                    image_barrier.subresourceRange.baseArrayLayer = 0;
                    image_barrier.subresourceRange.layerCount = 1;

                    image_barriers.push_back(image_barrier);

                    break;

                case resource_t::type::BUFFER:
                    VkBufferMemoryBarrier buffer_barrier = {};
                    buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                    buffer_barrier.srcAccessMask = barrier.access_before_barrier;
                    buffer_barrier.dstAccessMask = barrier.access_after_barrier;
                    buffer_barrier.srcQueueFamilyIndex = render_engine.get_queue_family_index(barrier.source_queue);
                    buffer_barrier.dstQueueFamilyIndex = render_engine.get_queue_family_index(barrier.destination_queue);
                    buffer_barrier.buffer = barrier.resource_to_barrier->buffer;
                    buffer_barrier.offset = barrier.buffer_memory_barrier.offset;
                    buffer_barrier.size = barrier.buffer_memory_barrier.size;

                    buffer_barriers.push_back(buffer_barrier);

                    break;
            }
        }

        vkCmdPipelineBarrier(cmds,
                             stages_before_barrier,
                             stages_after_barrier,
                             0,
                             0,
                             nullptr,
                             buffer_barriers.size(),
                             buffer_barriers.data(),
                             image_barriers.size(),
                             image_barriers.data());
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

    void vulkan_command_list::execute_command_lists(const std::vector<command_list*>& lists) {}
} // namespace nova::renderer
