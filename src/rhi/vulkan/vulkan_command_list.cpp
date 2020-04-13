#include "vulkan_command_list.hpp"

#include <minitrace.h>
#include <rx/core/log.h>
#include <string.h>
#include <vk_mem_alloc.h>

#include "nova_renderer/camera.hpp"
#include "nova_renderer/constants.hpp"
#include "nova_renderer/rhi/pipeline_create_info.hpp"

#include "vk_structs.hpp"
#include "vulkan_render_device.hpp"
#include "vulkan_resource_binder.hpp"
#include "vulkan_utils.hpp"

namespace nova::renderer::rhi {
    RX_LOG("VkCmdLst", logger);

    static VkIndexType to_vk_index_type(const IndexType index_type) {
        switch(index_type) {
            case IndexType::Uint16:
                return VK_INDEX_TYPE_UINT16;

            case IndexType::Uint32:
                return VK_INDEX_TYPE_UINT32;
        }

        return VK_INDEX_TYPE_UINT32;
    }

    VulkanRenderCommandList::VulkanRenderCommandList(const VkCommandBuffer cmds,
                                                     VulkanRenderDevice& render_device,
                                                     rx::memory::allocator& allocator)
        : cmds{cmds}, device{render_device}, internal_allocator{allocator}, descriptor_sets{allocator} {
        MTR_SCOPE("VulkanRenderCommandList", "VulkanRenderCommandList");
        // TODO: Put this begin info in the constructor parameters
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmds, &begin_info);
    }

    void VulkanRenderCommandList::set_debug_name(const rx::string& name) {
        const auto vk_name = vk::DebugUtilsObjectNameInfoEXT()
                                 .setObjectType(vk::ObjectType::eCommandBuffer)
                                 .setObjectHandle(reinterpret_cast<uint64_t>(cmds))
                                 .setPObjectName(name.data());

        device.device.setDebugUtilsObjectNameEXT(&vk_name, device.device_dynamic_loader);
    }

    void VulkanRenderCommandList::set_checkpoint(const rx::string& checkpoint_name) {
        if(device.has_nv_device_checkpoints) {
            // Save the checkpoint name to the render device, send the index of the checkpoint name to the driver
            const auto checkpoint_idx = device.save_checkpoint_name(checkpoint_name);

            device.device_dynamic_loader.vkCmdSetCheckpointNV(cmds, reinterpret_cast<void*>(checkpoint_idx));
        }
    }

    void VulkanRenderCommandList::bind_resources(RhiResourceBinder& binder) {
        MTR_SCOPE("VulkanRenderCommandList", "bind_resources");
        auto& vk_binder = static_cast<VulkanResourceBinder&>(binder);
        const auto& sets = vk_binder.get_sets();
        const auto& layout = vk_binder.get_layout();

        vkCmdBindDescriptorSets(cmds,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                static_cast<VkPipelineLayout>(layout),
                                0,
                                static_cast<uint32_t>(sets.size()),
                                reinterpret_cast<const VkDescriptorSet*>(sets.data()),
                                0,
                                nullptr);
    }

    void VulkanRenderCommandList::resource_barriers(const PipelineStage stages_before_barrier,
                                                    const PipelineStage stages_after_barrier,
                                                    const rx::vector<RhiResourceBarrier>& barriers) {
        MTR_SCOPE("VulkanRenderCommandList", "resource_barriers");
        rx::vector<VkBufferMemoryBarrier> buffer_barriers{internal_allocator};
        buffer_barriers.reserve(barriers.size());

        rx::vector<VkImageMemoryBarrier> image_barriers{internal_allocator};
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
                    image_barrier.srcQueueFamilyIndex = device.get_queue_family_index(barrier.source_queue);
                    image_barrier.dstQueueFamilyIndex = device.get_queue_family_index(barrier.destination_queue);
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
                    buffer_barrier.srcQueueFamilyIndex = device.get_queue_family_index(barrier.source_queue);
                    buffer_barrier.dstQueueFamilyIndex = device.get_queue_family_index(barrier.destination_queue);
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

    void VulkanRenderCommandList::copy_buffer(const RhiBuffer& destination_buffer,
                                              const mem::Bytes destination_offset,
                                              const RhiBuffer& source_buffer,
                                              const mem::Bytes source_offset,
                                              const mem::Bytes num_bytes) {
        MTR_SCOPE("VulkanRenderCommandList", "copy_buffer");
        VkBufferCopy copy;
        copy.srcOffset = source_offset.b_count();
        copy.dstOffset = destination_offset.b_count();
        copy.size = num_bytes.b_count();
        const auto& vk_destination_buffer = static_cast<const VulkanBuffer&>(destination_buffer);
        const auto& vk_source_buffer = static_cast<const VulkanBuffer&>(source_buffer);

        vkCmdCopyBuffer(cmds, vk_source_buffer.buffer, vk_destination_buffer.buffer, 1, &copy);
    }

    void VulkanRenderCommandList::execute_command_lists(const rx::vector<RhiRenderCommandList*>& lists) {
        MTR_SCOPE("VulkanRenderCommandList", "execute_command_lists");
        rx::vector<VkCommandBuffer> buffers{internal_allocator};
        buffers.reserve(lists.size());

        lists.each_fwd([&](RhiRenderCommandList* list) {
            auto* vk_list = dynamic_cast<VulkanRenderCommandList*>(list);
            buffers.push_back(vk_list->cmds);
        });

        vkCmdExecuteCommands(cmds, static_cast<uint32_t>(buffers.size()), buffers.data());
    }

    void VulkanRenderCommandList::set_camera(const Camera& camera) {
        MTR_SCOPE("VulkanRenderCommandList", "set_camera");
        camera_index = camera.index;

        vkCmdPushConstants(cmds, static_cast<VkPipelineLayout>(device.standard_pipeline_layout), VK_SHADER_STAGE_ALL, 0, sizeof(uint32_t), &camera_index);
    }

    void VulkanRenderCommandList::begin_renderpass(RhiRenderpass& renderpass, const RhiFramebuffer& framebuffer) {
        MTR_SCOPE("VulkanRenderCommandList", "begin_renderpass");
        auto& vk_renderpass = static_cast<VulkanRenderpass&>(renderpass);
        const auto& vk_framebuffer = static_cast<const VulkanFramebuffer&>(framebuffer);

        current_render_pass = &vk_renderpass;

        rx::vector<VkClearValue> clear_values{internal_allocator, vk_framebuffer.num_attachments};

        VkRenderPassBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        begin_info.renderPass = vk_renderpass.pass;
        begin_info.framebuffer = vk_framebuffer.framebuffer;
        begin_info.renderArea = {{0, 0}, {static_cast<uint32_t>(framebuffer.size.x), static_cast<uint32_t>(framebuffer.size.y)}};
        begin_info.clearValueCount = vk_framebuffer.num_attachments;
        begin_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(cmds, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanRenderCommandList::end_renderpass() {
        MTR_SCOPE("VulkanRenderCommandList", "end_renderpass");
        vkCmdEndRenderPass(cmds);

        current_render_pass = nullptr;
    }

    void VulkanRenderCommandList::set_material_index(uint32_t index) {
        vkCmdPushConstants(cmds, static_cast<VkPipelineLayout>(device.standard_pipeline_layout), VK_SHADER_STAGE_ALL, sizeof(uint32_t), sizeof(uint32_t), &index);
    }

    void VulkanRenderCommandList::set_pipeline(const RhiPipeline& state) {
        MTR_SCOPE("VulkanRenderCommandList", "set_pipeline");

        const auto& vk_pipeline = static_cast<const VulkanPipeline&>(state);

        if(current_render_pass != nullptr) {
            auto* pipeline = current_render_pass->cached_pipelines.find(vk_pipeline.state.name);
            if(pipeline == nullptr) {
                const auto pipeline_result = device.compile_pipeline_state(vk_pipeline, *current_render_pass, internal_allocator);
                if(pipeline_result) {
                    pipeline = current_render_pass->cached_pipelines.insert(vk_pipeline.state.name, *pipeline_result);

                } else {
                    logger->error("Could not compile pipeline %s", vk_pipeline.state.name);
                    return;
                }
            }

            if(pipeline != nullptr) {
                MTR_SCOPE("VulkanCommandList", "vkCmdBindPipeline");
                vkCmdBindPipeline(cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<VkPipeline>(*pipeline));
            }

        } else {
            logger->error("Cannot use a pipeline state when not in a renderpass");
        }
    }

    void VulkanRenderCommandList::bind_vertex_buffers(const rx::vector<RhiBuffer*>& buffers) {
        MTR_SCOPE("VulkanRenderCommandList", "bind_vertex_buffers");
        rx::vector<VkBuffer> vk_buffers{internal_allocator};
        vk_buffers.reserve(buffers.size());

        rx::vector<VkDeviceSize> offsets{internal_allocator};
        offsets.reserve(buffers.size());
        for(uint32_t i = 0; i < buffers.size(); i++) {
            offsets.push_back(i);
            const auto* vk_buffer = static_cast<const VulkanBuffer*>(buffers[i]);
            vk_buffers.push_back(vk_buffer->buffer);
        }

        vkCmdBindVertexBuffers(cmds, 0, static_cast<uint32_t>(vk_buffers.size()), vk_buffers.data(), offsets.data());
    }

    void VulkanRenderCommandList::bind_index_buffer(const RhiBuffer& buffer, const IndexType index_type) {
        MTR_SCOPE("VulkanRenderCommandList", "bind_index_buffer");
        const auto& vk_buffer = static_cast<const VulkanBuffer&>(buffer);

        vkCmdBindIndexBuffer(cmds, vk_buffer.buffer, 0, to_vk_index_type(index_type));
    }

    void VulkanRenderCommandList::draw_indexed_mesh(const uint32_t num_indices, const uint32_t offset, const uint32_t num_instances) {
        MTR_SCOPE("VulkanRenderCommandList", "draw_indexed_mesh");
        vkCmdDrawIndexed(cmds, num_indices, num_instances, offset, 0, 0);
    }

    void VulkanRenderCommandList::set_scissor_rect(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) {
        MTR_SCOPE("VulkanRenderCommandList", "set_scissor_rect");
        VkRect2D scissor_rect = {{static_cast<int32_t>(x), static_cast<int32_t>(y)}, {width, height}};
        vkCmdSetScissor(cmds, 0, 1, &scissor_rect);
    }

    void VulkanRenderCommandList::upload_data_to_image(const RhiImage& image,
                                                       const size_t width,
                                                       const size_t height,
                                                       const size_t bytes_per_pixel,
                                                       const RhiBuffer& staging_buffer,
                                                       const void* data) {
        MTR_SCOPE("VulkanRenderCommandList", "upload_data_to_image");
        const auto& vk_image = static_cast<const VulkanImage&>(image);
        const auto& vk_buffer = static_cast<const VulkanBuffer&>(staging_buffer);

        memcpy(vk_buffer.allocation_info.pMappedData, data, width * height * bytes_per_pixel);

        VkBufferImageCopy image_copy{};
        if(!vk_image.is_depth_tex) {
            image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        } else {
            logger->error("Can not upload data to depth images");
        }
        image_copy.imageSubresource.layerCount = 1;
        image_copy.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};

        vkCmdCopyBufferToImage(cmds, vk_buffer.buffer, vk_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy);
    }

    void VulkanRenderCommandList::cleanup_resources() {
        MTR_SCOPE("VulkanCommandList", "cleanup_resources");

        device.return_standard_descriptor_sets(descriptor_sets);

        descriptor_sets.clear();
    }
} // namespace nova::renderer::rhi
