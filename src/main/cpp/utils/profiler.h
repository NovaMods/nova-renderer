/*! 
 * \author gold1 
 * \date 30-Aug-17.
 */

#ifndef RENDERER_PROFILER_H
#define RENDERER_PROFILER_H

#include <unordered_map>
#include <string>
#include <ctime>

namespace nova {
    const int NUM_SAMPLES = 120;

    struct profiler_data {
        std::time_t start_time;

        // Using a ring buffer because it's fast
        std::time_t last_durations[NUM_SAMPLES];
        int cur_write_pos = 0;
    };

    /*!
     * \brief A simple namespace to hold profiling functions
     */
    class profiler {
    public:
        static void start(std::string name);
        static void end(std::string name);
        static void log_all_profiler_data();

    private:
        static std::unordered_map<std::string, profiler_data> data;
    };
}

#endif //RENDERER_PROFILER_H
