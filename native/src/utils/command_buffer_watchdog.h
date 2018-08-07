/*!
 * \author ddubois 
 * \date 06-Aug-18.
 */

#ifndef NOVA_RENDERER_COMMAND_BUFFER_WATCHDOG_H
#define NOVA_RENDERER_COMMAND_BUFFER_WATCHDOG_H

#include <vulkan/vulkan.hpp>
#include <chrono>
#include <thread>
#include <mutex>
#include "../render/vulkan/render_context.h"

namespace nova {
    struct watched_fence {
        vk::Fence fence;
        std::string name;
        std::chrono::high_resolution_clock::time_point start_time;

        watched_fence(const vk::Fence& fence, std::string name, const std::chrono::high_resolution_clock::time_point& start_time) :
                fence(fence), name(std::move(name)), start_time(start_time) {}
    };

    /*!
     * \brief Watchdog thread that can listen for and report on if command buffers have finished executing
     */
    class command_buffer_watchdog {
    public:
        static void initialize(const vk::Device& device);

        /*!
         * \brief Adds the fence to the list of fences to watch. It'll be polled every time the watchdog updates
         *
         * \param fence The fence to watch for completion of
         * \param name A human-readable name for this fence
         */
        static void add_watch(const std::string &name, const vk::Fence &fence);

        void operator()();

    private:
        static command_buffer_watchdog* instance;

        static std::vector<watched_fence> watched_fences;
        static std::mutex watched_fences_lock;

        static std::thread runner;

        vk::Device device;

        void tick();

        explicit command_buffer_watchdog(const vk::Device& device);
    };
}

#endif //NOVA_RENDERER_COMMAND_BUFFER_WATCHDOG_H
