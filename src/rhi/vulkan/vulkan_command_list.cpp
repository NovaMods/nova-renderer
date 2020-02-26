#include "vulkan_command_list.hpp"

#include <rx/core/log.h>
#include <string.h>
#include <vk_mem_alloc.h>

#include "vk_structs.hpp"
#include "vulkan_render_device.hpp"
#include "vulkan_utils.hpp"

namespace nova::renderer::rhi {
    RX_LOG("VkCmdLst", logger);
    VkIndexType to_vk_index_type(const IndexType index_type) {
        switch(index_type) {
            case IndexType::Uint16:
                return VK_INDEX_TYPE_UINT16;

            case IndexType::Uint32:
                [[fallthrough]];
            default:
                return VK_INDEX_TYPE_UINT32;
        }
    }

    VulkanRenderCommandList::VulkanRenderCommandList(VkCommandBuffer cmds, const VulkanRenderDevice* render_device)
        : cmds(cmds), render_device(*render_device) {

        // TODO: Put this begin info in the constructor parameters
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmds, &begin_info);
    }

    void VulkanRenderCommandList::set_debug_name(const rx::string& name) {
        VkDebugUtilsObjectNameInfoEXT vk_name{};
        vk_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        vk_name.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
        vk_name.objectHandle = reinterpret_cast<uint64_t>(cmds);
        vk_name.pObjectName = name.data();

        render_device.vkSetDebugUtilsObjectNameEXT(render_device.device, &vk_name);
    }

    void VulkanRenderCommandList::resource_barriers(const PipelineStage stages_before_barrier,
                                              const PipelineStage stages_after_barrier,
                                              const rx::vector<RhiResourceBarrier>& barriers) {
        rx::vector<VkBufferMemoryBarrier> buffer_barriers;
        buffer_barriers.reserve(barriers.size());

        rx::vector<VkImageMemoryBarrier> image_barriers;
        image_barriers.reserve(barriers.size());

        barriers.each_fwd([&](const RhiResourceBarrier& barrier) {
            switch(barrier.resource_to_barrier->type) {
                case ResourceType::Image: {
                    const auto* image = static_cast<VulkanImage*>(barrier.resource_to_barrier);

                    VkImageMemoryBarrier image_barrier = {};
                    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    image_barrier.srcAccessMask = to_vk_access_flags(barrier.access_before_barrier);
                    image_barrier.dstAccessMask = to_vk_access_flags(barrier.access_after_barrier);
                    image_barrier.oldLayout = to_vk_image_layout(barrier.old_state);
                    image_barrier.newLayout = to_vk_image_layout(barrier.new_state);
                    image_barrier.srcQueueFamilyIndex = render_device.get_queue_family_index(barrier.source_queue);
                    image_barrier.dstQueueFamilyIndex = render_device.get_queue_family_index(barrier.destination_queue);
                    image_barrier.image = image->image;
                    image_barrier.subresourceRange.aspectMask = static_cast<VkImageAspectFlags>(barrier.image_memory_barrier.aspect);
                    image_barrier.subresourceRange.baseMipLevel = 0; // TODO: Something smarter with mips
                    image_barrier.subresourceRange.levelCount = 1;
                    image_barrier.subresourceRange.baseArrayLayer = 0;
                    image_barrier.subresourceRange.layerCount = 1;

                    image_barriers.push_back(image_barrier);
                } break;

                case ResourceType::Buffer: {
                    const auto* buffer = static_cast<VulkanBuffer*>(barrier.resource_to_barrier);

                    VkBufferMemoryBarrier buffer_barrier = {};
                    buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                    buffer_barrier.srcAccessMask = to_vk_access_flags(barrier.access_before_barrier);
                    buffer_barrier.dstAccessMask = to_vk_access_flags(barrier.access_after_barrier);
                    buffer_barrier.srcQueueFamilyIndex = render_device.get_queue_family_index(barrier.source_queue);
                    buffer_barrier.dstQueueFamilyIndex = render_device.get_queue_family_index(barrier.destination_queue);
                    buffer_barrier.buffer = buffer->buffer;
                    buffer_barrier.offset = barrier.buffer_memory_barrier.offset.b_count();
                    buffer_barrier.size = barrier.buffer_memory_barrier.size.b_count();

                    buffer_barriers.push_back(buffer_barrier);
                } break;
            }
        });

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

    void VulkanRenderCommandList::copy_buffer(RhiBuffer* destination_buffer,
                                        const mem::Bytes destination_offset,
                                        RhiBuffer* source_buffer,
                                        const mem::Bytes source_offset,
                                        const mem::Bytes num_bytes) {
        VkBufferCopy copy;
        copy.srcOffset = source_offset.b_count();
        copy.dstOffset = destination_offset.b_count();
        copy.size = num_bytes.b_count();
        auto* vk_destination_buffer = static_cast<VulkanBuffer*>(destination_buffer);
        auto* vk_source_buffer = static_cast<VulkanBuffer*>(source_buffer);

        // TODO: fix the crash on this line
        vkCmdCopyBuffer(cmds, vk_source_buffer->buffer, vk_destination_buffer->buffer, 1, &copy);
    }

    void VulkanRenderCommandList::execute_command_lists(const rx::vector<RhiRenderCommandList*>& lists) {
        rx::vector<VkCommandBuffer> buffers;
        buffers.reserve(lists.size());

        lists.each_fwd([&](RhiRenderCommandList* list) {
            auto* vk_list = dynamic_cast<VulkanRenderCommandList*>(list);
            buffers.push_back(vk_list->cmds);
        });

        vkCmdExecuteCommands(cmds, static_cast<uint32_t>(buffers.size()), buffers.data());
    }

    void VulkanRenderCommandList::set_camera(const Camera& camera) {
        
    }

    void VulkanRenderCommandList::begin_renderpass(RhiRenderpass* renderpass, RhiFramebuffer* framebuffer) {
        auto* vk_renderpass = static_cast<VulkanRenderpass*>(renderpass);
        auto* vk_framebuffer = static_cast<VulkanFramebuffer*>(framebuffer);

        // FIXME: @dethraid this has been changed by @janrupf to fix a SIGSEGV
        //       and to match the number of clear values as used below in the
        //      VkRenderPassBeginInfo, however, there was another comment about
        //      required changes here before:
        //
        // // TODO: Store this somewhere better
        // // TODO: Get max framebuffer attachments from GPU
        rx::vector<VkClearValue> CLEAR_VALUES(vk_framebuffer->num_attachments);

        VkRenderPassBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        begin_info.renderPass = vk_renderpass->pass;
        begin_info.framebuffer = vk_framebuffer->framebuffer;
        begin_info.renderArea = {{0, 0}, {static_cast<uint32_t>(framebuffer->size.x), static_cast<uint32_t>(framebuffer->size.y)}};
        begin_info.clearValueCount = vk_framebuffer->num_attachments;
        begin_info.pClearValues = CLEAR_VALUES.data();

        // Nova _always_ records command lists in parallel for each renderpass
        vkCmdBeginRenderPass(cmds, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanRenderCommandList::end_renderpass() { vkCmdEndRenderPass(cmds); }

    void VulkanRenderCommandList::bind_pipeline(const RhiPipeline* pipeline) {
        const auto* vk_pipeline = static_cast<const VulkanPipeline*>(pipeline);
        vkCmdBindPipeline(cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline->pipeline);
    }

    void VulkanRenderCommandList::bind_descriptor_sets(const rx::vector<RhiDescriptorSet*>& descriptor_sets,
                                                 const RhiPipelineInterface* pipeline_interface) {
        const auto* vk_interface = static_cast<const VulkanPipelineInterface*>(pipeline_interface);

        for(uint32_t i = 0; i < descriptor_sets.size(); i++) {
            const auto* vk_set = static_cast<const VulkanDescriptorSet*>(descriptor_sets[i]);

            // logger(rx::log::level::k_verbose, "Binding descriptor set %x", vk_set->descriptor_set);

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

    void VulkanRenderCommandList::bind_vertex_buffers(const rx::vector<RhiBuffer*>& buffers) {
        rx::vector<VkBuffer> vk_buffers;
        vk_buffers.reserve(buffers.size());

        rx::vector<VkDeviceSize> offsets;
        offsets.reserve(buffers.size());
        for(uint32_t i = 0; i < buffers.size(); i++) {
            offsets.push_back(i);
            const auto* vk_buffer = static_cast<const VulkanBuffer*>(buffers[i]);
            vk_buffers.push_back(vk_buffer->buffer);
        }

        vkCmdBindVertexBuffers(cmds, 0, static_cast<uint32_t>(vk_buffers.size()), vk_buffers.data(), offsets.data());
    }

    void VulkanRenderCommandList::bind_index_buffer(const RhiBuffer* buffer, const IndexType index_type) {
        const auto* vk_buffer = static_cast<const VulkanBuffer*>(buffer);

        vkCmdBindIndexBuffer(cmds, vk_buffer->buffer, 0, to_vk_index_type(index_type));
    }

    void VulkanRenderCommandList::draw_indexed_mesh(const uint32_t num_indices, const uint32_t offset, const uint32_t num_instances) {
        vkCmdDrawIndexed(cmds, num_indices, num_instances, offset, 0, 0);
    }

    void VulkanRenderCommandList::set_scissor_rect(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) {
        VkRect2D scissor_rect = {{static_cast<int32_t>(x), static_cast<int32_t>(y)}, {width, height}};
        vkCmdSetScissor(cmds, 0, 1, &scissor_rect);
    }

    void VulkanRenderCommandList::upload_data_to_image(
        RhiImage* image, const size_t width, const size_t height, const size_t bytes_per_pixel, RhiBuffer* staging_buffer, const void* data) {
        auto* vk_image = static_cast<VulkanImage*>(image);
        auto* vk_buffer = static_cast<VulkanBuffer*>(staging_buffer);

        memcpy(vk_buffer->allocation_info.pMappedData, data, width * height * bytes_per_pixel);

        VkBufferImageCopy image_copy{};
        if(!vk_image->is_depth_tex) {
            image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        } else {
            logger(rx::log::level::k_error, "Can not upload data to depth images");
        }
        image_copy.imageSubresource.layerCount = 1;
        image_copy.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};

        vkCmdCopyBufferToImage(cmds, vk_buffer->buffer, vk_image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy);
    }
} // namespace nova::renderer::rhi
