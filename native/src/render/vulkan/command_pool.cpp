/*!
 * \author ddubois 
 * \date 17-Oct-17.
 */

#include <easylogging++.h>
#include "command_pool.h"
#include "render_context.h"
#include "../nova_renderer.h"
#include "../../utils/command_buffer_watchdog.h"

namespace nova {
    command_pool::command_pool(vk::Device device, uint32_t queue_family_index, uint32_t num_threads) {
        this->device = device;
        vk::CommandPoolCreateInfo command_pool_create_info = {};
        // Implicitly reset the command buffer when vkBeginCommandBuffer is called
        command_pool_create_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        command_pool_create_info.queueFamilyIndex = queue_family_index;

        command_pools.resize(num_threads);
        for(uint32_t i = 0; i < num_threads; i++) {
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

        buffer.pool_idx = thread_idx;

        return buffer;
    }

    void command_pool::free(command_buffer &buf) {
        device.freeCommandBuffers(command_pools[buf.pool_idx], 1, &buf.buffer);
        device.destroyFence(buf.fences[0]);
        device.destroyFence(buf.fences[1]);
    }

    command_pool::~command_pool() {
        for(auto& cp : command_pools) {
            device.destroyCommandPool(cp);
        }
    }

    void command_buffer::begin_as_single_commend() {
        vk::CommandBufferBeginInfo begin_info = {};
        begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        buffer.begin(begin_info);
    }

    void command_buffer::end_as_single_command(const std::string& execution_context) {
        auto context = nova_renderer::instance->get_render_context();

        buffer.end();

        vk::SubmitInfo submit_info = {};
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &buffer;

        auto& graphics_queue = context->graphics_queue;

        context->device.resetFences(1, fences);

        graphics_queue.submit(1, &submit_info, fences[0]);
        LOG(TRACE) << "Submitted buffer, it'll signal the fence when done";

        command_buffer_watchdog::get_instance().add_watch(execution_context, fences[0]);

        std::vector<vk::Fence> wait_fences;
        wait_fences.push_back(fences[0]);
        context->device.waitForFences(wait_fences, VK_TRUE, ~0ull);
        LOG(TRACE) << "Waited for the fence";
    }
}
