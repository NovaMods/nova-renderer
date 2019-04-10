/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#include "vulkan_command_list.hpp"
#include "vulkan_render_engine.hpp"
#include "vulkan_utils.hpp"

#include "vk_structs.hpp"

namespace nova::renderer::rhi {
    vulkan_command_list::vulkan_command_list(VkCommandBuffer cmds, const vk_render_engine& render_engine)
        : cmds(cmds), render_engine(render_engine) {}

    void vulkan_command_list::resource_barriers(const pipeline_stage_flags stages_before_barrier,
                                                const pipeline_stage_flags stages_after_barrier,
                                                const std::vector<resource_barrier_t>& barriers) {
        std::vector<VkBufferMemoryBarrier> buffer_barriers;
        buffer_barriers.reserve(barriers.size());

        std::vector<VkImageMemoryBarrier> image_barriers;
        image_barriers.reserve(barriers.size());

        for(const resource_barrier_t& barrier : barriers) {
            switch(barrier.resource_to_barrier->type) {
                case resource_t::type_t::IMAGE: {
                    vk_image_t* image = static_cast<vk_image_t*>(barrier.resource_to_barrier);

                    VkImageMemoryBarrier image_barrier = {};
                    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    image_barrier.srcAccessMask = barrier.access_before_barrier;
                    image_barrier.dstAccessMask = barrier.access_after_barrier;
                    image_barrier.oldLayout = to_vk_layout(barrier.initial_state);
                    image_barrier.newLayout = to_vk_layout(barrier.final_state);
                    image_barrier.srcQueueFamilyIndex = render_engine.get_queue_family_index(barrier.source_queue);
                    image_barrier.dstQueueFamilyIndex = render_engine.get_queue_family_index(barrier.destination_queue);
                    image_barrier.image = image->image;
                    image_barrier.subresourceRange.aspectMask = barrier.image_memory_barrier.aspect;
                    image_barrier.subresourceRange.baseMipLevel = 0; // TODO: Something smarter with mips
                    image_barrier.subresourceRange.levelCount = 1;
                    image_barrier.subresourceRange.baseArrayLayer = 0;
                    image_barrier.subresourceRange.layerCount = 1;

                    image_barriers.push_back(image_barrier);
                } break;

                case resource_t::type_t::BUFFER: {
                    vk_buffer_t* buffer = static_cast<vk_buffer_t*>(barrier.resource_to_barrier);

                    VkBufferMemoryBarrier buffer_barrier = {};
                    buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                    buffer_barrier.srcAccessMask = barrier.access_before_barrier;
                    buffer_barrier.dstAccessMask = barrier.access_after_barrier;
                    buffer_barrier.srcQueueFamilyIndex = render_engine.get_queue_family_index(barrier.source_queue);
                    buffer_barrier.dstQueueFamilyIndex = render_engine.get_queue_family_index(barrier.destination_queue);
                    buffer_barrier.buffer = buffer->buffer;
                    buffer_barrier.offset = barrier.buffer_memory_barrier.offset;
                    buffer_barrier.size = barrier.buffer_memory_barrier.size;

                    buffer_barriers.push_back(buffer_barrier);
                } break;
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

    void vulkan_command_list::copy_buffer(buffer_t* destination_buffer,
                                          const uint64_t destination_offset,
                                          buffer_t* source_buffer,
                                          const uint64_t source_offset,
                                          const uint64_t num_bytes) {
        VkBufferCopy copy = {};
        copy.srcOffset = source_offset;
        copy.dstOffset = destination_offset;
        copy.size = num_bytes;

        vk_buffer_t* vk_destination_buffer = static_cast<vk_buffer_t*>(destination_buffer);
        vk_buffer_t* vk_source_buffer = static_cast<vk_buffer_t*>(source_buffer);

        vkCmdCopyBuffer(cmds, vk_source_buffer->buffer, vk_destination_buffer->buffer, 1, &copy);
    }

    void vulkan_command_list::execute_command_lists(const std::vector<command_list_t*>& lists) {
        std::vector<VkCommandBuffer> buffers;
        buffers.reserve(lists.size());

        for(command_list_t* list : lists) {
            vulkan_command_list* vk_list = dynamic_cast<vulkan_command_list*>(list);
            buffers.push_back(vk_list->cmds);
        }

        vkCmdExecuteCommands(cmds, buffers.size(), buffers.data());
    }

    void vulkan_command_list::begin_renderpass(renderpass_t* renderpass, framebuffer_t* framebuffer) {
        vk_renderpass_t* vk_renderpass = static_cast<vk_renderpass_t*>(renderpass);
        vk_framebuffer_t* vk_framebuffer = static_cast<vk_framebuffer_t*>(framebuffer);

        VkRenderPassBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        begin_info.renderPass = vk_renderpass->pass;
        begin_info.framebuffer = vk_framebuffer->framebuffer;
        begin_info.renderArea = {static_cast<uint32_t>(framebuffer->size.x), static_cast<uint32_t>(framebuffer->size.y)};

        // Nova _always_ records command lists in parallel for each renderpass
        vkCmdBeginRenderPass(cmds, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }
} // namespace nova::renderer::rhi
