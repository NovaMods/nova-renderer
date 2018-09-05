//
// Created by ddubois on 9/4/18.
//

#include "vulkan_command_buffer.hpp"
#include "vulkan_resource_barrier_helpers.hpp"

namespace nova {

    vulkan_command_buffer::vulkan_command_buffer(VkDevice device, VkCommandPool command_pool, command_buffer_type type) :
            command_buffer_base(type), device(device) {
        VkCommandBufferAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.pNext = nullptr;
        allocate_info.commandPool = command_pool;
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandBufferCount = 1;

        vkAllocateCommandBuffers(device, &allocate_info, &buffer);

        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.pNext = nullptr;
        fence_create_info.flags = 0;

        vkCreateFence(device, &fence_create_info, nullptr, &completion_fence);
    }

    void vulkan_command_buffer::on_completion(std::function<void(void)> completion_handler) {
        // TODO: Figure out how to implement this
    }

    void vulkan_command_buffer::end_recording() {
        vkEndCommandBuffer(buffer);
    }

    void vulkan_command_buffer::reset() {
        vkResetCommandBuffer(buffer, 0);
    }

    void vulkan_command_buffer::resource_barrier(const std::vector<resource_barrier_data> &barriers) {
        std::vector<VkMemoryBarrier> memory_barriers;
        memory_barriers.reserve(barriers.size());

        std::vector<VkBufferMemoryBarrier> buffer_barriers;
        buffer_barriers.reserve(barriers.size());

        std::vector<VkImageMemoryBarrier> image_barriers;
        image_barriers.reserve(barriers.size());

        for(const resource_barrier_data& barrier_data : barriers) {
            VkAccessFlags source_access_flags = to_vk_access_flags(barrier_data.initial_layout);
            VkAccessFlags dest_access_flags = to_vk_access_flags(barrier_data.final_layout);


        }
    }
}
