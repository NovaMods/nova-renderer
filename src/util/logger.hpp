#include <utility>

/*!
 * \author ddubois
 * \date 21-Aug-18.
 */

#ifndef NOVA_RENDERER_LOGGER_HPP
#define NOVA_RENDERER_LOGGER_HPP

#include <functional>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

#ifdef ERROR
#undef ERROR
#endif

namespace nova::renderer {
    enum log_level { TRACE, DEBUG, INFO, WARN, ERROR, FATAL, MAX_LEVEL };

    class _log_stream;
    /*!
     * \brief A logger interface that can be implemented for whatever game Nova uses
     */
    class logger {
    public:
        static logger instance;

        explicit logger() = default;

        void add_log_handler(log_level level, const std::function<void(std::string)>& log_handler);

        void log(log_level level, const std::string& msg);

        _log_stream log(log_level level) const;

    private:
        std::unordered_map<log_level, std::function<void(std::string)>> log_handlers;
        std::mutex log_lock;
    };

    // Allow stream logging
    // Todo: Fix this junk
    class _log_stream : public std::stringstream {
    private:
        log_level level;

    public:
        explicit _log_stream(log_level level);

        _log_stream(const _log_stream& other) = delete;
        _log_stream& operator=(const _log_stream& other) = delete;

        _log_stream(_log_stream&& other) noexcept;
        _log_stream& operator=(_log_stream&& other) noexcept;

        ~_log_stream() override;
    };

} // namespace nova

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NOVA_LOG(LEVEL) ::nova::logger::instance.log(::nova::log_level::LEVEL)

#endif // NOVA_RENDERER_LOGGER_HPP
