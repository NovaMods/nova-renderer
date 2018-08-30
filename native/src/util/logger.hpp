#include <utility>

/*!
 * \author ddubois 
 * \date 21-Aug-18.
 */

#ifndef NOVA_RENDERER_LOGGER_HPP
#define NOVA_RENDERER_LOGGER_HPP

#include <string>
#include <functional>
#include <unordered_map>
#include <sstream>

#ifdef ERROR
    #undef ERROR
#endif

namespace nova {
    enum log_level {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        MAX_LEVEL
    };

    class __log_stream;
    /*!
     * \brief A logger interface that can be implemented for whatever game Nova uses
     */
    class logger {
    public:
        static logger instance;

        void add_log_handler(log_level level, const std::function<void(std::string)> &log_handler);

        void log(log_level level, const std::string &msg) const;

        __log_stream log(log_level level) const;

    private:
        std::unordered_map<log_level, std::function<void(std::string)>> log_handlers;
    };

    // Allow stream logging
    class __log_stream : public std::stringstream {
    private:
        const logger _logger;
        const log_level level;

    public:
        explicit __log_stream(logger logger, log_level level);

        ~__log_stream();
    };

#define NOVA_LOG(LEVEL) nova::logger::instance.log(nova::log_level::LEVEL)
}


#endif //NOVA_RENDERER_LOGGER_HPP
