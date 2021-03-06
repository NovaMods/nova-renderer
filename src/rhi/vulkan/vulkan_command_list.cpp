#include "vulkan_command_list.hpp"

#include <Tracy.hpp>
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
    RX_LOG("vk::CmdLst", logger);
    vk::IndexType to_vk_index_type(const IndexType index_type) {
        switch(index_type) {
            case IndexType::Uint16:
                return VK_INDEX_TYPE_UINT16;

            case IndexType::Uint32:
                [[fallthrough]];
            default:
                return VK_INDEX_TYPE_UINT32;
        }
    }

    VulkanRenderCommandList::VulkanRenderCommandList(vk::CommandBuffer cmds,
                                                     VulkanRenderDevice& render_device,
                                                     rx::memory::allocator& allocator)
        : cmds(cmds), device(render_device), allocator(allocator), descriptor_sets{&allocator} {
        ZoneScoped;        // TODO: Put this begin info in the constructor parameters
        vk::CommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmds, &begin_info);
    }

    void VulkanRenderCommandList::set_debug_name(const std::string& name) {
        vk::DebugUtilsObjectNameInfoEXT vk_name{};
        vk_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        vk_name.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
        vk_name.objectHandle = reinterpret_cast<uint64_t>(cmds);
        vk_name.pObjectName = name.data();

        device.vkSetDebugUtilsObjectNameEXT(device.device, &vk_name);
    }

    void VulkanRenderCommandList::bind_material_resources(RhiBuffer* camera_buffer,
                                                          RhiBuffer* material_buffer,
                                                          RhiSampler* point_sampler,
                                                          RhiSampler* bilinear_sampler,
                                                          RhiSampler* trilinear_sampler,
                                                          const std::vector<RhiImage*>& textures,
                                                          rx::memory::allocator& allocator) {
        const auto set = device.get_next_standard_descriptor_set();

        const auto* vk_camera_buffer = static_cast<VulkanBuffer*>(camera_buffer);
        const auto camera_buffer_write = vk::DescriptorBufferInfo()
                                             .setOffset(0)
                                             .setRange(vk_camera_buffer->size.b_count())
                                             .setBuffer(vk_camera_buffer->buffer);
        const auto camera_buffer_descriptor_type = vk_camera_buffer->size < device.gpu.props.limits.maxUniformBufferRange ?
                                                       vk::DescriptorType::eUniformBuffer :
                                                       vk::DescriptorType::eStorageBuffer;

        const auto* vk_material_buffer = static_cast<VulkanBuffer*>(material_buffer);
        const auto material_buffer_write = vk::DescriptorBufferInfo()
                                               .setOffset(0)
                                               .setRange(material_buffer->size.b_count())
                                               .setBuffer(vk_material_buffer->buffer);
        const auto material_buffer_descriptor_type = material_buffer->size < device.gpu.props.limits.maxUniformBufferRange ?
                                                         vk::DescriptorType::eUniformBuffer :
                                                         vk::DescriptorType::eStorageBuffer;

        const auto* vk_point_sampler = static_cast<VulkanSampler*>(point_sampler);
        const auto point_sampler_write = vk::DescriptorImageInfo().setSampler(vk_point_sampler->sampler);

        const auto* vk_bilinear_sampler = static_cast<VulkanSampler*>(bilinear_sampler);
        const auto bilinear_sampler_write = vk::DescriptorImageInfo().setSampler(vk_bilinear_sampler->sampler);

        const auto* vk_trilinear_sampler = static_cast<VulkanSampler*>(trilinear_sampler);
        const auto trilinear_sampler_write = vk::DescriptorImageInfo().setSampler(vk_trilinear_sampler->sampler);

        std::vector<vk::DescriptorImageInfo> vk_textures{&allocator};
        vk_textures.reserve(textures.size());

        textures.each_fwd([&](const RhiImage* image) {
            const auto* vk_image = static_cast<const VulkanImage*>(image);
            vk_textures.emplace_back(
                vk::DescriptorImageInfo().setImageView(vk_image->image_view).setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal));
        });

        const auto writes = std::array{
            vk::WriteDescriptorSet()
                .setDstSet(set)
                .setDstBinding(0)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(camera_buffer_descriptor_type)
                .setPBufferInfo(&camera_buffer_write),
            vk::WriteDescriptorSet()
                .setDstSet(set)
                .setDstBinding(1)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(material_buffer_descriptor_type)
                .setPBufferInfo(&material_buffer_write),
            vk::WriteDescriptorSet()
                .setDstSet(set)
                .setDstBinding(2)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eSampler)
                .setPImageInfo(&point_sampler_write),
            vk::WriteDescriptorSet()
                .setDstSet(set)
                .setDstBinding(3)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eSampler)
                .setPImageInfo(&bilinear_sampler_write),
            vk::WriteDescriptorSet()
                .setDstSet(set)
                .setDstBinding(4)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eSampler)
                .setPImageInfo(&trilinear_sampler_write),
            vk::WriteDescriptorSet()
                .setDstSet(set)
                .setDstBinding(5)
                .setDstArrayElement(0)
                .setDescriptorCount(static_cast<uint32_t>(vk_textures.size()))
                .setDescriptorType(vk::DescriptorType::eSampledImage)
                .setPImageInfo(vk_textures.data()),
        };

        device.device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

        vkCmdBindDescriptorSets(cmds,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                device.standard_pipeline_layout,
                                0,
                                1,
                                reinterpret_cast<const vk::DescriptorSet*>(&set),
                                0,
                                nullptr);

        descriptor_sets.emplace_back(set);
    }

    void VulkanRenderCommandList::bind_resources(RhiResourceBinder& binder) {
        auto& vk_binder = static_cast<VulkanResourceBinder&>(binder);
        const auto& sets = vk_binder.get_sets();
        const auto& layout = vk_binder.get_layout();

        vkCmdBindDescriptorSets(cmds,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                layout,
                                0,
                                static_cast<uint32_t>(sets.size()),
                                reinterpret_cast<const vk::DescriptorSet*>(sets.data()),
                                0,
                                nullptr);
    }

    void VulkanRenderCommandList::resource_barriers(const PipelineStage stages_before_barrier,
                                                    const PipelineStage stages_after_barrier,
                                                    const std::vector<RhiResourceBarrier>& barriers) {
        ZoneScoped;        std::vector<vk::BufferMemoryBarrier> buffer_barriers{&allocator};
        buffer_barriers.reserve(barriers.size());

        std::vector<vk::ImageMemoryBarrier> image_barriers{&allocator};
        image_barriers.reserve(barriers.size());

        barriers.each_fwd([&](const RhiResourceBarrier& barrier) {
            switch(barrier.resource_to_barrier->type) {
                case ResourceType::Image: {
                    const auto* image = static_cast<VulkanImage*>(barrier.resource_to_barrier);

                    vk::ImageMemoryBarrier image_barrier = {};
                    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    image_barrier.srcAccessMask = to_vk_access_flags(barrier.access_before_barrier);
                    image_barrier.dstAccessMask = to_vk_access_flags(barrier.access_after_barrier);
                    image_barrier.oldLayout = to_vk_image_layout(barrier.old_state);
                    image_barrier.newLayout = to_vk_image_layout(barrier.new_state);
                    image_barrier.srcQueueFamilyIndex = device.get_queue_family_index(barrier.source_queue);
                    image_barrier.dstQueueFamilyIndex = device.get_queue_family_index(barrier.destination_queue);
                    image_barrier.image = image->image;
                    image_barrier.subresourceRange.aspectMask = static_cast<vk::ImageAspectFlags>(barrier.image_memory_barrier.aspect);
                    image_barrier.subresourceRange.baseMipLevel = 0; // TODO: Something smarter with mips
                    image_barrier.subresourceRange.levelCount = 1;
                    image_barrier.subresourceRange.baseArrayLayer = 0;
                    image_barrier.subresourceRange.layerCount = 1;

                    image_barriers.push_back(image_barrier);
                } break;

                case ResourceType::Buffer: {
                    const auto* buffer = static_cast<VulkanBuffer*>(barrier.resource_to_barrier);

                    vk::BufferMemoryBarrier buffer_barrier = {};
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
                             static_cast<vk::PipelineStageFlags>(stages_before_barrier),
                             static_cast<vk::PipelineStageFlags>(stages_after_barrier),
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
        ZoneScoped;        vk::BufferCopy copy;
        copy.srcOffset = source_offset.b_count();
        copy.dstOffset = destination_offset.b_count();
        copy.size = num_bytes.b_count();
        auto* vk_destination_buffer = static_cast<VulkanBuffer*>(destination_buffer);
        auto* vk_source_buffer = static_cast<VulkanBuffer*>(source_buffer);

        // TODO: fix the crash on this line
        vkCmdCopyBuffer(cmds, vk_source_buffer->buffer, vk_destination_buffer->buffer, 1, &copy);
    }

    void VulkanRenderCommandList::execute_command_lists(const std::vector<RhiRenderCommandList*>& lists) {
        ZoneScoped;        std::vector<vk::CommandBuffer> buffers{&allocator};
        buffers.reserve(lists.size());

        lists.each_fwd([&](RhiRenderCommandList* list) {
            auto* vk_list = dynamic_cast<VulkanRenderCommandList*>(list);
            buffers.push_back(vk_list->cmds);
        });

        vkCmdExecuteCommands(cmds, static_cast<uint32_t>(buffers.size()), buffers.data());
    }

    void VulkanRenderCommandList::set_camera(const Camera& camera) {
        ZoneScoped;        camera_index = camera.index;

        vkCmdPushConstants(cmds, device.standard_pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(uint32_t), &camera_index);
    }

    void VulkanRenderCommandList::begin_renderpass(RhiRenderpass* renderpass, RhiFramebuffer* framebuffer) {
        ZoneScoped;        auto* vk_renderpass = static_cast<VulkanRenderpass*>(renderpass);
        auto* vk_framebuffer = static_cast<VulkanFramebuffer*>(framebuffer);

        current_render_pass = vk_renderpass;

        std::vector<vk::ClearValue> clear_values{&allocator, vk_framebuffer->num_attachments};

        vk::RenderPassBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        begin_info.renderPass = vk_renderpass->pass;
        begin_info.framebuffer = vk_framebuffer->framebuffer;
        begin_info.renderArea = {{0, 0}, {static_cast<uint32_t>(framebuffer->size.x), static_cast<uint32_t>(framebuffer->size.y)}};
        begin_info.clearValueCount = vk_framebuffer->num_attachments;
        begin_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(cmds, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanRenderCommandList::end_renderpass() {
        ZoneScoped;        vkCmdEndRenderPass(cmds);

        current_render_pass = nullptr;
    }

    void VulkanRenderCommandList::set_material_index(uint32_t index) {
        vkCmdPushConstants(cmds, device.standard_pipeline_layout, VK_SHADER_STAGE_ALL, sizeof(uint32_t), sizeof(uint32_t), &index);
    }

    void VulkanRenderCommandList::set_pipeline(const RhiPipeline& state) {
        ZoneScoped;
        const auto& vk_pipeline = static_cast<const VulkanPipeline&>(state);

        if(current_render_pass != nullptr) {
            auto* pipeline = current_render_pass->cached_pipelines.find(vk_pipeline.state.name);
            if(pipeline == nullptr) {
                const auto pipeline_result = device.compile_pipeline_state(vk_pipeline, *current_render_pass, allocator);
                if(pipeline_result) {
                    pipeline = current_render_pass->cached_pipelines.insert(vk_pipeline.state.name, *pipeline_result);

                } else {
                    logger->error("Could not compile pipeline %s", vk_pipeline.state.name);
                    return;
                }
            }

            if(pipeline != nullptr) {
                vkCmdBindPipeline(cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<vk::Pipeline>(*pipeline));
            }

        } else {
            logger->error("Cannot use a pipeline state when not in a renderpass");
        }
    }

    void VulkanRenderCommandList::bind_descriptor_sets(const std::vector<RhiDescriptorSet*>& descriptor_sets,
                                                       const RhiPipelineInterface* pipeline_interface) {
        ZoneScoped;        const auto* vk_interface = static_cast<const VulkanPipelineInterface*>(pipeline_interface);

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

    void VulkanRenderCommandList::bind_vertex_buffers(const std::vector<RhiBuffer*>& buffers) {
        ZoneScoped;        std::vector<vk::Buffer> vk_buffers{&allocator};
        vk_buffers.reserve(buffers.size());

        std::vector<vk::DeviceSize> offsets{&allocator};
        offsets.reserve(buffers.size());
        for(uint32_t i = 0; i < buffers.size(); i++) {
            offsets.push_back(i);
            const auto* vk_buffer = static_cast<const VulkanBuffer*>(buffers[i]);
            vk_buffers.push_back(vk_buffer->buffer);
        }

        vkCmdBindVertexBuffers(cmds, 0, static_cast<uint32_t>(vk_buffers.size()), vk_buffers.data(), offsets.data());
    }

    void VulkanRenderCommandList::bind_index_buffer(const RhiBuffer* buffer, const IndexType index_type) {
        ZoneScoped;        const auto* vk_buffer = static_cast<const VulkanBuffer*>(buffer);

        vkCmdBindIndexBuffer(cmds, vk_buffer->buffer, 0, to_vk_index_type(index_type));
    }

    void VulkanRenderCommandList::draw_indexed_mesh(const uint32_t num_indices, const uint32_t offset, const uint32_t num_instances) {
        ZoneScoped;        vkCmdDrawIndexed(cmds, num_indices, num_instances, offset, 0, 0);
    }

    void VulkanRenderCommandList::set_scissor_rect(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) {
        ZoneScoped;        vk::Rect2D scissor_rect = {{static_cast<int32_t>(x), static_cast<int32_t>(y)}, {width, height}};
        vkCmdSetScissor(cmds, 0, 1, &scissor_rect);
    }

    void VulkanRenderCommandList::upload_data_to_image(RhiImage* image,
                                                       const size_t width,
                                                       const size_t height,
                                                       const size_t bytes_per_pixel,
                                                       RhiBuffer* staging_buffer,
                                                       const void* data) {
        ZoneScoped;        auto* vk_image = static_cast<VulkanImage*>(image);
        auto* vk_buffer = static_cast<VulkanBuffer*>(staging_buffer);

        memcpy(vk_buffer->allocation_info.pMappedData, data, width * height * bytes_per_pixel);

        vk::BufferImageCopy image_copy{};
        if(!vk_image->is_depth_tex) {
            image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        } else {
            logger->error("Can not upload data to depth images");
        }
        image_copy.imageSubresource.layerCount = 1;
        image_copy.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};

        vkCmdCopyBufferToImage(cmds, vk_buffer->buffer, vk_image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy);
    }

    void VulkanRenderCommandList::cleanup_resources() {
        ZoneScoped;
        device.return_standard_descriptor_sets(descriptor_sets);

        descriptor_sets.clear();
    }
} // namespace nova::renderer::rhi
