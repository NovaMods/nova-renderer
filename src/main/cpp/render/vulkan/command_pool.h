/*!
 * \author ddubois 
 * \date 17-Oct-17.
 */

#ifndef RENDERER_COMMAND_POOL_H
#define RENDERER_COMMAND_POOL_H


#include <vulkan/vulkan.hpp>
#include <vector>

#define NUM_FRAME_DATA 2

namespace nova {
    struct command_buffer {
        vk::CommandBuffer buffer;
        vk::Fence fences[NUM_FRAME_DATA];
    };

    /*!
     * \brief A wrapper around a Vulkan command pool that lets you allocate command buffers from multiple threads
     */
    class command_pool {
    public:
        /*!
         * \brief Creates this command_pool object, creating one command buffer pool per worker thread
         * \param device The VkDevice to use
         * \param num_threads The number of threads that the system uses. If you're using more than 255 threads then
         * your computer's too powerful
         */
        command_pool(vk::Device device, uint32_t queue_family_index, uint32_t num_threads);

        /*!
         * \brief Retrieves a new command buffer
         * \param thread_idx the index of the thread that wants a command buffer
         * \return A command buffer
         */
        command_buffer get_command_buffer(uint8_t thread_idx);
    private:
        vk::Device device;

        std::vector<vk::CommandPool> command_pools;
    };
}

#endif //RENDERER_COMMAND_POOL_H
