//
// Created by ddubois on 9/4/18.
//

#include "vulkan_command_buffer.hpp"
#include "vulkan_resource_barrier_helpers.hpp"
#include "vulkan_opaque_types.hpp"
#include "../../util/logger.hpp"
#include "vulkan_utils.hpp"

namespace nova {

    vulkan_command_buffer::vulkan_command_buffer(VkDevice device, VkCommandPool command_pool, command_buffer_type type) :
            command_buffer_base(type), device(device) {
        VkCommandBufferAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.pNext = nullptr;
        allocate_info.commandPool = command_pool;
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandBufferCount = 1;

        NOVA_THROW_IF_VK_ERROR(vkAllocateCommandBuffers(device, &allocate_info, &buffer), command_buffer_exception);

        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.pNext = nullptr;
        fence_create_info.flags = 0;

        NOVA_THROW_IF_VK_ERROR(vkCreateFence(device, &fence_create_info, nullptr, &completion_fence), command_buffer_exception);
    }

    void vulkan_command_buffer::on_completion(std::function<void(void)> completion_handler) {
        // TODO: Figure out how to implement this
    }

    void vulkan_command_buffer::start_recording() {
        VkCommandBufferBeginInfo begin_info;
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.pNext = nullptr;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        begin_info.pInheritanceInfo = nullptr;

        NOVA_THROW_IF_VK_ERROR(vkBeginCommandBuffer(buffer, &begin_info), command_buffer_exception);
    }

    void vulkan_command_buffer::end_recording() {
        vkEndCommandBuffer(buffer);
    }

    void vulkan_command_buffer::reset() {
        vkResetCommandBuffer(buffer, 0);
    }

    void vulkan_command_buffer::resource_barrier(stage_flags source_stage_mask, stage_flags dest_stage_mask,
                                                 const std::vector<resource_barrier_data>& memory_barriers,
                                                 const std::vector<buffer_barrier_data>& buffer_barriers,
                                                 const std::vector<image_barrier_data>& image_barriers) {
        std::vector<VkMemoryBarrier> vk_memory_barriers;
        vk_memory_barriers.reserve(memory_barriers.size());

        VkAccessFlags source_access_mask = to_vk_access_flags(source_stage_mask);
        VkAccessFlags dest_access_mask = to_vk_access_flags(dest_stage_mask);

        for(const resource_barrier_data& barrier_data : memory_barriers) {
            VkMemoryBarrier memory_barrier = {};
            memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            memory_barrier.pNext = nullptr;
            memory_barrier.dstAccessMask = source_access_mask;
            memory_barrier.dstAccessMask = dest_access_mask;

            vk_memory_barriers.push_back(memory_barrier);
        }

        std::vector<VkBufferMemoryBarrier> vk_buffer_barriers;
        vk_buffer_barriers.reserve(buffer_barriers.size());

        for(const buffer_barrier_data &barrier_data : buffer_barriers) {
            VkBufferMemoryBarrier buffer_barrier = {};
            buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            buffer_barrier.pNext = nullptr;
            buffer_barrier.srcAccessMask = source_access_mask;
            buffer_barrier.dstAccessMask = dest_access_mask;
            buffer_barrier.buffer = barrier_data.resource_to_barrier->resource.buffer;
            buffer_barrier.offset = (VkDeviceSize)barrier_data.offset;
            buffer_barrier.size = (VkDeviceSize)barrier_data.range;
            buffer_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            buffer_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            vk_buffer_barriers.push_back(buffer_barrier);
        }

        std::vector<VkImageMemoryBarrier> vk_image_barriers;
        vk_image_barriers.reserve(image_barriers.size());

        for(const image_barrier_data& barrier_data : image_barriers) {
            VkImageMemoryBarrier image_barrier = {};
            image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            image_barrier.pNext = nullptr;
            image_barrier.dstAccessMask = dest_access_mask;
            image_barrier.srcAccessMask = source_access_mask;
            image_barrier.image = barrier_data.resource_to_barrier->resource.image;
            image_barrier.oldLayout = to_vk_image_layout(barrier_data.initial_layout);
            image_barrier.newLayout = to_vk_image_layout(barrier_data.final_layout);
            image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_barrier.subresourceRange.baseMipLevel = 0;
            image_barrier.subresourceRange.levelCount = 1;
            image_barrier.subresourceRange.baseArrayLayer = 0;
            image_barrier.subresourceRange.layerCount = 1;

            vk_image_barriers.push_back(image_barrier);
        }

        VkPipelineStageFlags source_pipeline_flags = to_vk_stage_flags(source_stage_mask);
        VkPipelineStageFlags dest_pipeline_flags = to_vk_stage_flags(dest_stage_mask);

        vkCmdPipelineBarrier(buffer, source_pipeline_flags, dest_pipeline_flags, 0,
                             static_cast<uint32_t>(vk_memory_barriers.size()), vk_memory_barriers.data(),
                             static_cast<uint32_t>(vk_buffer_barriers.size()), vk_buffer_barriers.data(),
                             static_cast<uint32_t>(vk_image_barriers.size()), vk_image_barriers.data());
    }

    void vulkan_command_buffer::clear_render_target(iframebuffer *framebuffer_to_clear, glm::vec4 &clear_color) {

    }

    void vulkan_command_buffer::set_render_target(iframebuffer *render_target) {

    }

    bool vulkan_command_buffer::is_finished() const {
        return false;
    }

    void vulkan_command_buffer::wait_until_completion() const {

    }

    VkCommandBuffer vulkan_command_buffer::get_vk_buffer() {
        return buffer;
    }
}
