/*!
 * \author ddubois 
 * \date 06-Aug-18.
 */

#include <easylogging++.h>
#include "command_buffer_watchdog.h"
#include "../render/nova_renderer.h"

namespace nova {
    command_buffer_watchdog* command_buffer_watchdog::instance;


    void command_buffer_watchdog::initialize(const vk::Device& device) {
        instance = new command_buffer_watchdog(device);
    }

    command_buffer_watchdog::command_buffer_watchdog(const vk::Device &device) :
            device(device), runner([this](){(*this)();}) {}

    void command_buffer_watchdog::tick() {
        const auto polling_time = std::chrono::high_resolution_clock::now();

        watched_fences_lock.lock();

        std::vector<std::string> finished_watches;
        for(const auto& watch : watched_fences) {
            const auto status = device.getFenceStatus(watch.fence);
            const auto time_since_watch_start = std::chrono::duration_cast<std::chrono::milliseconds>(polling_time - watch.start_time);
            if(status == vk::Result::eSuccess) {
                LOG(TRACE) << "Watch " << watch.name << " finished after " << time_since_watch_start.count() << "ms";
                finished_watches.push_back(watch.name);

            } else if(status == vk::Result::eNotReady) {
                LOG(TRACE) << "Still waiting for " << watch.name << " - it's been " << time_since_watch_start.count() << "ms";
            }
        }

        const auto& removed_itr = std::remove_if(watched_fences.begin(), watched_fences.end(), [finished_watches, this](const watched_fence& watch){
            return std::find(finished_watches.begin(), finished_watches.end(), watch.name) != finished_watches.end();
        });
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

    void command_buffer_watchdog::remove_watch(const std::string &name) {
        const auto removal_time = std::chrono::high_resolution_clock::now();

        watched_fences_lock.lock();
        const auto& removed_elems_iter = std::remove_if(watched_fences.begin(), watched_fences.end(), [&](const auto& watch){
            if(watch.name == name) {
                const auto time_since_watch_start = std::chrono::duration_cast<std::chrono::milliseconds>(removal_time - watch.start_time);
                LOG(TRACE) << "Watch " << watch.name << " removed after " << time_since_watch_start.count() << "ms";
                return true;
            }
            return false;
        });
        watched_fences.erase(removed_elems_iter, watched_fences.end());
        watched_fences_lock.unlock();
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    void command_buffer_watchdog::operator()() {
        while(true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            tick();
        }
    }

    command_buffer_watchdog &command_buffer_watchdog::get_instance() {
        if(instance == nullptr) {
            initialize(nova_renderer::instance->get_render_context()->device);
        }
        return *instance;
    }

#pragma clang diagnostic pop

}
