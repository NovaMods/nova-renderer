/*! 
 * \author gold1 
 * \date 30-Aug-17.
 */

#ifndef RENDERER_PROFILER_H
#define RENDERER_PROFILER_H

#include <unordered_map>
#include <string>
#include <chrono>
#include <easylogging++.h>

namespace nova {
    const int NUM_SAMPLES = 120;

    struct profiler_data {
        std::chrono::high_resolution_clock::time_point start_time;

        // Using a ring buffer because it's fast
        std::chrono::high_resolution_clock::duration total_duration;
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
