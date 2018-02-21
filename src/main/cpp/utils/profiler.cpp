/*! 
 * \author gold1 
 * \date 30-Aug-17.
 */

#include <cstring>
#include "profiler.h"
#include <easylogging++.h>

namespace nova {
    std::unordered_map<std::string, profiler_data> profiler::data;

    void profiler::start(std::string name) {
        auto section_itr = data.find(name);
        if(section_itr == data.end()) {
            data[name] = profiler_data();
        }

        auto &cur_profiler_data = data[name];
        cur_profiler_data.start_time = std::chrono::high_resolution_clock::now();
    }

    void profiler::end(std::string name) {
        auto &cur_profiler_data = data[name];
        auto duration = std::chrono::high_resolution_clock::now() - cur_profiler_data.start_time;
        cur_profiler_data.total_duration += duration;
    }

    void profiler::log_all_profiler_data() {
        std::stringstream ss;
        for(const auto& item : data) {
            const auto& cur_profiler_data = item.second;

            ss << "Profiled section " << item.first << " has taken an total of " << double(std::chrono::duration_cast<std::chrono::nanoseconds>(cur_profiler_data.total_duration).count()) / 1000000.0f << "ms to execute since the game began\n";
        }

        //LOG_EVERY_N(100, DEBUG) << ss.str();
    }
}
