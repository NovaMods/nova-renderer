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

    VulkanRenderCommandList::VulkanRenderCommandList(VkCommandBuffer cmds,
                                                     VulkanRenderDevice& render_device,
                                                     rx::memory::allocator& allocator)
        : cmds(cmds), device(render_device), allocator(allocator) {
        MTR_SCOPE("VulkanRenderCommandList", "VulkanRenderCommandList");
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

        device.vkSetDebugUtilsObjectNameEXT(device.device, &vk_name);
    }

    void VulkanRenderCommandList::bind_resources(RhiBuffer* material_buffer,
                                                 RhiSampler* point_sampler,
                                                 RhiSampler* bilinear_sampler,
                                                 RhiSampler* trilinear_sampler,
                                                 const rx::vector<RhiImage*>& textures,
                                                 rx::memory::allocator& allocator) {
        const auto set = device.get_next_standard_descriptor_set();

        const auto* vk_buffer = static_cast<VulkanBuffer*>(material_buffer);
        const auto material_buffer_write = vk::DescriptorBufferInfo()
                                               .setOffset(0)
                                               .setRange(MATERIAL_BUFFER_SIZE.b_count())
                                               .setBuffer(vk_buffer->buffer);

        const auto* vk_point_sampler = static_cast<VulkanSampler*>(point_sampler);
        const auto point_sampler_write = vk::DescriptorImageInfo().setSampler(vk_point_sampler->sampler);

        const auto* vk_bilinear_sampler = static_cast<VulkanSampler*>(bilinear_sampler);
        const auto bilinear_sampler_write = vk::DescriptorImageInfo().setSampler(vk_bilinear_sampler->sampler);

        const auto* vk_trilinear_sampler = static_cast<VulkanSampler*>(trilinear_sampler);
        const auto trilinear_sampler_write = vk::DescriptorImageInfo().setSampler(vk_trilinear_sampler->sampler);

        rx::vector<vk::DescriptorImageInfo> vk_textures{&allocator};
        vk_textures.reserve(textures.size());

        textures.each_fwd([&](const RhiImage* image) {
            const auto* vk_image = static_cast<const VulkanImage*>(image);
            vk_textures.emplace_back(
                vk::DescriptorImageInfo().setImageView(vk_image->image_view).setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal));
        });

        const auto writes = rx::array{
            vk::WriteDescriptorSet()
                .setDstSet(set)
                .setDstBinding(0)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setPBufferInfo(&material_buffer_write),
            vk::WriteDescriptorSet()
                .setDstSet(set)
                .setDstBinding(1)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eSampler)
                .setPImageInfo(&point_sampler_write),
            vk::WriteDescriptorSet()
                .setDstSet(set)
                .setDstBinding(2)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eSampler)
                .setPImageInfo(&bilinear_sampler_write),
            vk::WriteDescriptorSet()
                .setDstSet(set)
                .setDstBinding(3)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eSampler)
                .setPImageInfo(&trilinear_sampler_write),
            vk::WriteDescriptorSet()
                .setDstSet(set)
                .setDstBinding(4)
                .setDstArrayElement(0)
                .setDescriptorCount(vk_textures.size())
                .setDescriptorType(vk::DescriptorType::eSampledImage)
                .setPImageInfo(vk_textures.data()),
        };

        device.device.updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);

        vkCmdBindDescriptorSets(cmds,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                device.standard_pipeline_layout,
                                0,
                                1,
                                reinterpret_cast<const VkDescriptorSet*>(&set),
                                0,
                                nullptr);

        descriptor_sets.emplace_back(set);
    }

    void VulkanRenderCommandList::resource_barriers(const PipelineStage stages_before_barrier,
                                                    const PipelineStage stages_after_barrier,
                                                    const rx::vector<RhiResourceBarrier>& barriers) {
        MTR_SCOPE("VulkanRenderCommandList", "resource_barriers");
        rx::vector<VkBufferMemoryBarrier> buffer_barriers{&allocator};
        buffer_barriers.reserve(barriers.size());

        rx::vector<VkImageMemoryBarrier> image_barriers{&allocator};
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

    void VulkanRenderCommandList::copy_buffer(RhiBuffer* destination_buffer,
                                              const mem::Bytes destination_offset,
                                              RhiBuffer* source_buffer,
                                              const mem::Bytes source_offset,
                                              const mem::Bytes num_bytes) {
        MTR_SCOPE("VulkanRenderCommandList", "copy_buffer");
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
        MTR_SCOPE("VulkanRenderCommandList", "execute_command_lists");
        rx::vector<VkCommandBuffer> buffers{&allocator};
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

        if(current_layout != VK_NULL_HANDLE) {
            vkCmdPushConstants(cmds, current_layout, VK_SHADER_STAGE_ALL, 0, sizeof(uint32_t), &camera_index);
        }
    }

    void VulkanRenderCommandList::begin_renderpass(RhiRenderpass* renderpass, RhiFramebuffer* framebuffer) {
        MTR_SCOPE("VulkanRenderCommandList", "begin_renderpass");
        auto* vk_renderpass = static_cast<VulkanRenderpass*>(renderpass);
        auto* vk_framebuffer = static_cast<VulkanFramebuffer*>(framebuffer);

        current_render_pass = vk_renderpass;

        rx::vector<VkClearValue> clear_values{&allocator, vk_framebuffer->num_attachments};

        VkRenderPassBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        begin_info.renderPass = vk_renderpass->pass;
        begin_info.framebuffer = vk_framebuffer->framebuffer;
        begin_info.renderArea = {{0, 0}, {static_cast<uint32_t>(framebuffer->size.x), static_cast<uint32_t>(framebuffer->size.y)}};
        begin_info.clearValueCount = vk_framebuffer->num_attachments;
        begin_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(cmds, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanRenderCommandList::end_renderpass() {
        MTR_SCOPE("VulkanRenderCommandList", "end_renderpass");
        vkCmdEndRenderPass(cmds);

        current_render_pass = nullptr;
    }

    void VulkanRenderCommandList::set_pipeline_state(const RhiGraphicsPipelineState& state) {
        MTR_SCOPE("VulkanRenderCommandList", "bind_pipeline");

        if(current_render_pass != nullptr) {
            auto* pipeline = current_render_pass->cached_pipelines.find(state.name);
            if(pipeline == nullptr) {
                const auto pipeline_result = device.create_pipeline(state, current_render_pass->pass, allocator);
                if(pipeline_result) {
                    current_render_pass->cached_pipelines.insert(state.name, *pipeline_result);
                    pipeline = current_render_pass->cached_pipelines.find(state.name);

                } else {
                    logger(rx::log::level::k_error, "Could not compile pipeline %s", state.name);
                    return;
                }
            }

            if(pipeline != nullptr) {
                vkCmdBindPipeline(cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
            }

        } else {
            logger(rx::log::level::k_error, "Cannot use a pipeline state when not in a renderpass");
        }
    }

    void VulkanRenderCommandList::bind_descriptor_sets(const rx::vector<RhiDescriptorSet*>& descriptor_sets,
                                                       const RhiPipelineInterface* pipeline_interface) {
        MTR_SCOPE("VulkanRenderCommandList", "bind_descriptor_sets");
        const auto* vk_interface = static_cast<const VulkanPipelineInterface*>(pipeline_interface);

        for(uint32_t i = 0; i < descriptor_sets.size(); i++) {
            const auto* vk_set = static_cast<const VulkanDescriptorSet*>(descriptor_sets[i]);

            vkCmdBindDescriptorSets(cmds,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    device.standard_pipeline_layout,
                                    i,
                                    1,
                                    &vk_set->descriptor_set,
                                    0,
                                    nullptr);
        }
    }

    void VulkanRenderCommandList::bind_vertex_buffers(const rx::vector<RhiBuffer*>& buffers) {
        MTR_SCOPE("VulkanRenderCommandList", "bind_vertex_buffers");
        rx::vector<VkBuffer> vk_buffers{&allocator};
        vk_buffers.reserve(buffers.size());

        rx::vector<VkDeviceSize> offsets{&allocator};
        offsets.reserve(buffers.size());
        for(uint32_t i = 0; i < buffers.size(); i++) {
            offsets.push_back(i);
            const auto* vk_buffer = static_cast<const VulkanBuffer*>(buffers[i]);
            vk_buffers.push_back(vk_buffer->buffer);
        }

        vkCmdBindVertexBuffers(cmds, 0, static_cast<uint32_t>(vk_buffers.size()), vk_buffers.data(), offsets.data());
    }

    void VulkanRenderCommandList::bind_index_buffer(const RhiBuffer* buffer, const IndexType index_type) {
        MTR_SCOPE("VulkanRenderCommandList", "bind_index_buffer");
        const auto* vk_buffer = static_cast<const VulkanBuffer*>(buffer);

        vkCmdBindIndexBuffer(cmds, vk_buffer->buffer, 0, to_vk_index_type(index_type));
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

    void VulkanRenderCommandList::upload_data_to_image(RhiImage* image,
                                                       const size_t width,
                                                       const size_t height,
                                                       const size_t bytes_per_pixel,
                                                       RhiBuffer* staging_buffer,
                                                       const void* data) {
        MTR_SCOPE("VulkanRenderCommandList", "upload_data_to_image");
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
