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
            std::memset(data[name].last_durations, 0, NUM_SAMPLES);
        }

        auto &cur_profiler_data = data[name];
        cur_profiler_data.start_time = std::chrono::high_resolution_clock::now();
    }

    void profiler::end(std::string name) {
        auto &cur_profiler_data = data[name];
        auto duration = std::chrono::high_resolution_clock::now() - cur_profiler_data.start_time;
        cur_profiler_data.last_durations[cur_profiler_data.cur_write_pos] = duration;
        cur_profiler_data.cur_write_pos++;
        if(cur_profiler_data.cur_write_pos >= NUM_SAMPLES) {
            cur_profiler_data.cur_write_pos = 0;
        }
    }

    void profiler::log_all_profiler_data() {
        std::stringstream ss;
        for(const auto& item : data) {
            const auto& cur_profiler_data = item.second;

            long long int average_duration = 0;
            int count = cur_profiler_data.has_write_pos_reset ? NUM_SAMPLES : cur_profiler_data.cur_write_pos;
            for(int i = 0; i < count; i++) {
                average_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(cur_profiler_data.last_durations[i]).count();
            }

            if(count != 0) {
                average_duration /= count;
            }

            ss << "Profiled section " << item.first << " took an average of " << float(average_duration) / 1000000.0f << "ms to execute\n";
        }

        LOG_EVERY_N(100, DEBUG) << ss.str();
    }
}
