/*!
 * \author ddubois 
 * \date 01-Jul-18.
 */

#ifndef NOVA_PROFILER_PROFILER_H
#define NOVA_PROFILER_PROFILER_H

#include <string>
#include <ctime>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <mutex>

#include <optional>

#ifdef NOVA_PROFILER_HIGH_RES
#define NOVA_CLOCK std::chrono::high_resolution_clock
#else
#define NOVA_CLOCK std::chrono::system_clock
#endif

#ifndef NOVA_PROFILER_LOG_STREAM
#define NOVA_PROFILER_LOG_STREAM std::cerr
#endif

#ifndef NOVA_PROFILER_LOG_INCLUDE
#define NOVA_PROFILER_LOG_INCLUDE <iostream>
#endif

#define NOVA_PROFILER_SCOPE auto __nova_profiled_scope = nova::profiler::profiled_scope(__func__)

#define NOVA_PROFILER_FLUSH_TO_FILE(filename) nova::profiler::profiler_impl::get_instance().flush_to_file(filename)

namespace nova::profiler {

    /*!
     * \brief A single profiler datapoint. Can be for either a scope or a block
     */
    struct profiler_data {
        std::string name = "<name missing>";
        NOVA_CLOCK::time_point start_time;
        NOVA_CLOCK::time_point end_time;

        profiler_data* parent = nullptr;
        /*!
         * \brief A map from the scope name to all the samples taken in that scope
         *
         * The scope name is the name of every profiled scope which is a direct child to thic scope
         *
         * The samples are one sample for each time that scope is seen. A scope may be seen if
         */
        std::unordered_map<std::string, std::vector<profiler_data>> children;

        /*!
         * \brief Builds a string of this and all its children
         *
         * Example output:
         * nova_renderer::render_frame 98ms
         *   | camera::recalculate_frustum 5ms
         *   | nova_renderer::update_nova_ubos 35ms
         *       | nova_renderer::update_per_frame_ubos 10ms
         *       | nova_renderer::update_gui_model_matrices 20ms
         *   | glfw_window::end_frame 5ms
         *
         * \param level the indentation level of the current printing
         * \param prefix The prefix to append once foe each indentation level
         * \return A string of the profiler data
         */
        std::string to_string(int level = 0, const std::string& prefix = "  | ") const;

        /*!
         * \brief Returns the duration of this profiled datapoint
         * \return The duration of this datapoint
         */
        NOVA_CLOCK::duration get_duration() const;
    };

    /*!
     * \brief A RAII object that measures the time in a specific scope
     *
     * When this object is destructed, it grabs the profiler_impl singleton and saves its time there
     */
    class profiled_scope : public profiler_data {
    public:
        /*!
         * \brief Records the time this object was constructed
         * \param name The name of the scope you're profiling
         */
        explicit profiled_scope(std::string name);

        /*!
         * \brief Records the time that this object was destructed, saving that time to the profiler_impl
         */
        ~profiled_scope();
    };

    /*!
     * \brief A singleton that keeps track of all the profiler data
     *
     * This profiler was intended for use in a game, so you should call
     */
    class profiler_impl {
    public:
        static profiler_impl &get_instance();

        /*!
         * \brief Sets the current scope to the given scope
         *
         * If there is no current scope, adds the new profiler data to the root-level datas map
         *
         * \param datapoint The
         */
        void enter_scope(profiled_scope& datapoint);
        void exit_scope();

        void flush_to_file(const std::string& filename);

    private:
        static profiler_impl instance;

        std::mutex top_level_datas_lock;
        std::unordered_map<std::string, std::vector<profiler_data>> top_level_datas;

        std::mutex cur_data_lock;
        /*!
         * \brief The profiler data for the current thing being profiled
         */
        std::unordered_map<std::thread::id, profiler_data> cur_data;

        profiler_impl() = default;
    };
}

#endif //NOVA_PROFILER_PROFILER_H
