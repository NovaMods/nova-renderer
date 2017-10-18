/*!
 * \author ddubois 
 * \date 17-Oct-17.
 */

#include <easylogging++.h>
#include "command_pool.h"

nova::command_pool::command_pool(VkDevice device, uint32_t queue_family_index, uint8_t num_threads) {
    this->device = device;
    VkCommandPoolCreateInfo command_pool_create_info = {};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    // Implicitly reset the command buffer when vkBeginCommandBuffer is called
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    command_pool_create_info.queueFamilyIndex = queue_family_index;

    command_pools.resize(num_threads);
    for(int i = 0; i < num_threads; i++) {
        auto err = vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pools[i]);
        if(err != VK_SUCCESS) {
            LOG(FATAL) << "Could not create command pool with index " << i;
        }
    }
}

command_buffer nova::command_pool::get_command_buffer(uint8_t thread_idx) {
    command_buffer buffer = {};

    VkCommandBufferAllocateInfo command_buffer_allocation_info = {};
    command_buffer_allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    command_buffer_allocation_info.commandPool = command_pools[thread_idx];

    command_buffer_allocation_info.commandBufferCount = NUM_FRAME_DATA;

    auto err = vkAllocateCommandBuffers(device, &command_buffer_allocation_info, &buffer.buffer);
    if(err != VK_SUCCESS) {
        LOG(FATAL) << "Could not create command buffer for thread " << thread_idx;
    }

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    for(int i = 0; i < NUM_FRAME_DATA; i++) {
        err = vkCreateFence(device, &fenceCreateInfo, nullptr, &buffer.fences[i]);
        if(err != VK_SUCCESS) {
            LOG(FATAL) << "Could not create fence for frame data " << i << " for thread " << thread_idx;
        }
    }

    return buffer;
}
