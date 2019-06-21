/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#include "vulkan_command_list.hpp"
#include "vulkan_render_engine.hpp"
#include "vulkan_utils.hpp"

#include "vk_structs.hpp"

namespace nova::renderer::rhi {
    VulkanCommandList::VulkanCommandList(VkCommandBuffer cmds, const VulkanRenderEngine& render_engine)
        : cmds(cmds), render_engine(render_engine) {}

    void VulkanCommandList::resource_barriers(const PipelineStageFlags stages_before_barrier,
                                              const PipelineStageFlags stages_after_barrier,
                                              const std::vector<ResourceBarrier>& barriers) {
        std::vector<VkBufferMemoryBarrier> buffer_barriers;
        buffer_barriers.reserve(barriers.size());

        std::vector<VkImageMemoryBarrier> image_barriers;
        image_barriers.reserve(barriers.size());

        for(const ResourceBarrier& barrier : barriers) {
            switch(barrier.resource_to_barrier->type) {
                case ResourceType::Image: {
                    VulkanImage* image = static_cast<VulkanImage*>(barrier.resource_to_barrier);

                    VkImageMemoryBarrier image_barrier = {};
                    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    image_barrier.srcAccessMask = static_cast<VkAccessFlags>(barrier.access_before_barrier);
                    image_barrier.dstAccessMask = static_cast<VkAccessFlags>(barrier.access_after_barrier);
                    image_barrier.oldLayout = to_vk_layout(barrier.initial_state);
                    image_barrier.newLayout = to_vk_layout(barrier.final_state);
                    image_barrier.srcQueueFamilyIndex = render_engine.get_queue_family_index(barrier.source_queue);
                    image_barrier.dstQueueFamilyIndex = render_engine.get_queue_family_index(barrier.destination_queue);
                    image_barrier.image = image->image;
                    image_barrier.subresourceRange.aspectMask = static_cast<VkImageAspectFlags>(barrier.image_memory_barrier.aspect);
                    image_barrier.subresourceRange.baseMipLevel = 0; // TODO: Something smarter with mips
                    image_barrier.subresourceRange.levelCount = 1;
                    image_barrier.subresourceRange.baseArrayLayer = 0;
                    image_barrier.subresourceRange.layerCount = 1;

                    image_barriers.push_back(image_barrier);
                } break;

                case ResourceType::Buffer: {
                    VulkanBuffer* buffer = static_cast<VulkanBuffer*>(barrier.resource_to_barrier);

                    VkBufferMemoryBarrier buffer_barrier = {};
                    buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                    buffer_barrier.srcAccessMask = static_cast<VkAccessFlags>(barrier.access_before_barrier);
                    buffer_barrier.dstAccessMask = static_cast<VkAccessFlags>(barrier.access_after_barrier);
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
                             static_cast<VkPipelineStageFlags>(stages_before_barrier),
                             static_cast<VkPipelineStageFlags>(stages_after_barrier),
                             0,
                             0,
                             nullptr,
                             static_cast<uint32_t>(buffer_barriers.size()),
                             buffer_barriers.data(),
                             static_cast<uint32_t>(image_barriers.size()),
                             image_barriers.data());
    }

    void VulkanCommandList::copy_buffer(Buffer* destination_buffer,
                                        const uint64_t destination_offset,
                                        Buffer* source_buffer,
                                        const uint64_t source_offset,
                                        const uint64_t num_bytes) {
        VkBufferCopy copy = {};
        copy.srcOffset = source_offset;
        copy.dstOffset = destination_offset;
        copy.size = num_bytes;

        VulkanBuffer* vk_destination_buffer = static_cast<VulkanBuffer*>(destination_buffer);
        VulkanBuffer* vk_source_buffer = static_cast<VulkanBuffer*>(source_buffer);

        vkCmdCopyBuffer(cmds, vk_source_buffer->buffer, vk_destination_buffer->buffer, 1, &copy);
    }

    void VulkanCommandList::execute_command_lists(const std::vector<CommandList*>& lists) {
        std::vector<VkCommandBuffer> buffers;
        buffers.reserve(lists.size());

        for(CommandList* list : lists) {
            VulkanCommandList* vk_list = dynamic_cast<VulkanCommandList*>(list);
            buffers.push_back(vk_list->cmds);
        }

        vkCmdExecuteCommands(cmds, static_cast<uint32_t>(buffers.size()), buffers.data());
    }

    void VulkanCommandList::begin_renderpass(Renderpass* renderpass, Framebuffer* framebuffer) {
        VulkanRenderpass* vk_renderpass = static_cast<VulkanRenderpass*>(renderpass);
        VulkanFramebuffer* vk_framebuffer = static_cast<VulkanFramebuffer*>(framebuffer);

        VkRenderPassBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        begin_info.renderPass = vk_renderpass->pass;
        begin_info.framebuffer = vk_framebuffer->framebuffer;
        begin_info.renderArea = {0, 0, static_cast<uint32_t>(framebuffer->size.x), static_cast<uint32_t>(framebuffer->size.y)};

        // Nova _always_ records command lists in parallel for each renderpass
        vkCmdBeginRenderPass(cmds, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanCommandList::end_renderpass() { vkCmdEndRenderPass(cmds); }

    void VulkanCommandList::bind_pipeline(const Pipeline* pipeline) {
        const VulkanPipeline* vk_pipeline = static_cast<const VulkanPipeline*>(pipeline);
        vkCmdBindPipeline(cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline->pipeline);
    }

    void VulkanCommandList::bind_descriptor_sets(const std::vector<DescriptorSet*>& descriptor_sets,
                                                 const PipelineInterface* pipeline_interface) {
        const VulkanPipelineInterface* vk_interface = static_cast<const VulkanPipelineInterface*>(pipeline_interface);

        for(uint32_t i = 0; i < descriptor_sets.size(); i++) {
            const VulkanDescriptorSet* vk_set = static_cast<const VulkanDescriptorSet*>(descriptor_sets.at(i));
            vkCmdBindDescriptorSets(cmds,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    vk_interface->pipeline_layout,
                                    i,
                                    1,
                                    &vk_set->descriptor_set,
                                    0,
                                    nullptr);
        }
    }

    void VulkanCommandList::bind_vertex_buffers(const std::vector<Buffer*>& buffers) {
        std::vector<VkBuffer> vk_buffers;
        vk_buffers.reserve(buffers.size());

        std::vector<VkDeviceSize> offsets;
        offsets.reserve(buffers.size());
        for(uint32_t i = 0; i < buffers.size(); i++) {
            offsets.push_back(i);
            const VulkanBuffer* vk_buffer = static_cast<const VulkanBuffer*>(buffers.at(i));
            vk_buffers.push_back(vk_buffer->buffer);
        }

        vkCmdBindVertexBuffers(cmds, 0, vk_buffers.size(), vk_buffers.data(), offsets.data());
    }

    void VulkanCommandList::bind_index_buffer(const Buffer* buffer) {
        const VulkanBuffer* vk_buffer = static_cast<const VulkanBuffer*>(buffer);

        vkCmdBindIndexBuffer(cmds, vk_buffer->buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void VulkanCommandList::draw_indexed_mesh(const uint64_t num_indices, const uint64_t num_instances) {
        vkCmdDrawIndexed(cmds, num_indices, num_instances, 0, 0, 0);
    }
} // namespace nova::renderer::rhi
