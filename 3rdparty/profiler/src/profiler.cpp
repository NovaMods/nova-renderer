/*!
 * \author ddubois 
 * \date 01-Jul-18.
 */

#include <fstream>
#include <sstream>
#include "../include/nova/profiler.h"

#include NOVA_PROFILER_LOG_INCLUDE

namespace nova::profiler {
    profiler_impl profiler_impl::instance;

    std::string profiler_data::to_string(const int level, const std::string &prefix) const {
        constexpr double conversion_factor = static_cast<double>(std::chrono::milliseconds::duration::period::num) / NOVA_CLOCK::duration::period::den;

        auto duration = get_duration();

        std::stringstream ss;
        ss << name << " " << conversion_factor * duration.count() << "ms";
        return ss.str();
    }

    NOVA_CLOCK::duration profiler_data::get_duration() const {
        return end_time - start_time;
    }

   profiled_scope::profiled_scope(std::string name) {
        this->name = name;

        start_time = NOVA_CLOCK::now();

        profiler_impl::get_instance().enter_scope(*this);
    }

    profiled_scope::~profiled_scope() {
        end_time = NOVA_CLOCK::now();

        profiler_impl::get_instance().exit_scope();
    }

    profiler_impl &profiler_impl::get_instance() {
        return instance;
    }

    void profiler_impl::enter_scope(profiled_scope& datapoint) {
        const std::thread::id thread_id = std::this_thread::get_id();

        if(cur_data.find(thread_id) == cur_data.end()) {
            top_level_datas_lock.lock();
            top_level_datas[datapoint.name].push_back(datapoint);
            top_level_datas_lock.unlock();

        } else {
            cur_data_lock.lock();
            cur_data[thread_id].children[datapoint.name].push_back(datapoint);
            cur_data_lock.unlock();
            datapoint.parent = &cur_data[thread_id];
        }

        cur_data_lock.lock();
        cur_data[thread_id] = datapoint;
        cur_data_lock.unlock();
    }

    void profiler_impl::exit_scope() {
        const std::thread::id thread_id = std::this_thread::get_id();
        if(cur_data.find(thread_id) == cur_data.end()) {
            NOVA_PROFILER_LOG_STREAM << "Exiting scope when there's no current scope - all calls to `profiler_impl::exit_scope` MUST be proceeded by a call to `profiler_impl::enter_scope`";
            return;
        }

        cur_data_lock.lock();
        if (cur_data[thread_id].parent == nullptr) {
            cur_data.erase(thread_id);
        } else {
            cur_data[thread_id] = *cur_data[thread_id].parent;
        }
        cur_data_lock.unlock();
    }

    void profiler_impl::flush_to_file(const std::string& filename) {
        top_level_datas_lock.lock();
        std::ofstream of(filename, std::ios_base::app);
        if(!of.is_open()) {
            NOVA_PROFILER_LOG_STREAM << "Could not open profiler file " << filename << ". Deleting profiler data anyways";
            top_level_datas.clear();
            top_level_datas_lock.unlock();
            return;
        }
        of << "\n\n";

        NOVA_PROFILER_LOG_STREAM << "Saving data for " << top_level_datas.size() << " top-level scopes to file " << filename << std::endl;

        for(const auto& item : top_level_datas) {
            NOVA_PROFILER_LOG_STREAM << "Saving " << item.second.size() << " datapoints for scope " << item.first << std::endl;
            of << "Scope " << item.first << ":" << std::endl;

            for(const auto& datapoint : item.second) {
                NOVA_PROFILER_LOG_STREAM << "Saving datapoint " << datapoint.name << std::endl;

                of << datapoint.to_string() << std::endl;
            }
        }

        of.close();

        top_level_datas.clear();
        top_level_datas_lock.unlock();
    }
}
