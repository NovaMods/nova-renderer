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
        cur_profiler_data.start_time = std::time(nullptr);
    }

    void profiler::end(std::string name) {
        std::time_t end_time = std::time(nullptr);
        auto &cur_profiler_data = data[name];
        auto duration = end_time - cur_profiler_data.start_time;
        cur_profiler_data.last_durations[cur_profiler_data.cur_write_pos] = duration;
        cur_profiler_data.cur_write_pos++;
        if(cur_profiler_data.cur_write_pos >= NUM_SAMPLES) {
            cur_profiler_data.cur_write_pos = 0;
        }
    }

    void profiler::log_all_profiler_data() {
        for(const auto& item : data) {
            const auto& cur_profiler_data = item.second;

            double average_duration = 0;
            int num_samples = 0;
            for(int i = 0; i < NUM_SAMPLES; i++) {
                if(cur_profiler_data.last_durations[i] != 0) {
                    average_duration += cur_profiler_data.last_durations[i];
                    num_samples++;
                }
            }

            average_duration /= num_samples;

            LOG(DEBUG) << "Profiled section " << item.first << " took an average of " << average_duration << "ms to execute (" << num_samples << " samples)";
        }
    }
}
