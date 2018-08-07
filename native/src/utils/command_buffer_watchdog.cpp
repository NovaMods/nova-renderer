/*!
 * \author ddubois 
 * \date 06-Aug-18.
 */

#include <easylogging++.h>
#include "command_buffer_watchdog.h"

namespace nova {
    command_buffer_watchdog* command_buffer_watchdog::instance;
    std::vector<watched_fence> command_buffer_watchdog::watched_fences;
    std::mutex command_buffer_watchdog::watched_fences_lock;


    void command_buffer_watchdog::initialize(const vk::Device& device) {
        instance = new command_buffer_watchdog(device);
    }

    command_buffer_watchdog::command_buffer_watchdog(const vk::Device &device) : device(device) {
        runner = std::thread(*this);
    }

    void command_buffer_watchdog::tick() {
        const auto polling_time = std::chrono::high_resolution_clock::now();

        watched_fences_lock.lock();

        for(const auto& watch : watched_fences) {
            const auto status = device.getFenceStatus(watch.fence);
            if(status == vk::Result::eSuccess) {
                LOG(TRACE) << "Watch " << watch.name << " finished after " << (polling_time - watch.start_time).count() << "ms";

            } else if(status == vk::Result::eNotReady) {
                LOG(TRACE) << "Still waiting for " << watch.name;
            }
        }

        const auto& removed_itr = std::remove_if(watched_fences.begin(), watched_fences.end(),
                       [this](const watched_fence& watch){return device.getFenceStatus(watch.fence) == vk::Result::eSuccess;});
        watched_fences.erase(removed_itr, watched_fences.end());

        watched_fences_lock.unlock();
    }


    void command_buffer_watchdog::add_watch(const std::string &name, const vk::Fence &fence) {
        if(instance == nullptr) {
            LOG(WARNING) << "Cannot add watch " << name << " because the watchdog isn't started yet";
            return;
        }

        watched_fences_lock.lock();
        watched_fences.emplace_back(fence, name, std::chrono::high_resolution_clock::now());
        watched_fences_lock.unlock();
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    void command_buffer_watchdog::operator()() {
        while(true) {
            std::this_thread::sleep_for(std::chrono::high_resolution_clock::duration(1));

            tick();
        }
    }
#pragma clang diagnostic pop

}
