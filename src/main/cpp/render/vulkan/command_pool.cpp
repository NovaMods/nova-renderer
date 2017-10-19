/*!
 * \author ddubois 
 * \date 17-Oct-17.
 */

#include <easylogging++.h>
#include "command_pool.h"

namespace nova {
    command_pool::command_pool(vk::Device device, uint32_t queue_family_index, uint32_t num_threads) {
        this->device = device;
        vk::CommandPoolCreateInfo command_pool_create_info = {};
        // Implicitly reset the command buffer when vkBeginCommandBuffer is called
        command_pool_create_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        command_pool_create_info.queueFamilyIndex = queue_family_index;

        command_pools.resize(num_threads);
        for(int i = 0; i < num_threads; i++) {
            command_pools[i] = device.createCommandPool(command_pool_create_info);
        }
    }

    command_buffer command_pool::get_command_buffer(uint8_t thread_idx) {
        command_buffer buffer = {};

        vk::CommandBufferAllocateInfo command_buffer_allocation_info = {};
        command_buffer_allocation_info.level = vk::CommandBufferLevel::ePrimary;

        command_buffer_allocation_info.commandPool = command_pools[thread_idx];

        command_buffer_allocation_info.commandBufferCount = NUM_FRAME_DATA;

        buffer.buffer = device.allocateCommandBuffers(command_buffer_allocation_info)[0];

        vk::FenceCreateInfo fenceCreateInfo = {};

        for(int i = 0; i < NUM_FRAME_DATA; i++) {
            buffer.fences[i] = device.createFence(fenceCreateInfo);
        }

        return buffer;
    }
};
