#pragma once

#include <functional>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

// Undefine ERROR because we use it as an enum name and windows.h defines it which messes with our code
#ifdef ERROR
#undef ERROR
#endif

namespace nova::renderer {
    // ReSharper disable CppInconsistentNaming
    enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL, MAX_LEVEL };

    class _log_stream;
    /*!
     * \brief A logger interface that can be implemented for whatever game Nova uses
     */
    class Logger {
    public:
        static Logger instance;

        explicit Logger() = default;

        void add_log_handler(LogLevel level, const std::function<void(std::string)>& log_handler);

        void log(LogLevel level, const std::string& msg);

        _log_stream log(LogLevel level) const;

    private:
        std::unordered_map<LogLevel, std::function<void(std::string)>> log_handlers;
        std::mutex log_lock;
    };

    // Allow stream logging
    // Todo: Fix this junk
    class _log_stream : public std::stringstream {
    private:
		LogLevel level;

    public:
        explicit _log_stream(LogLevel level);

        _log_stream(const _log_stream& other) = delete;
        _log_stream& operator=(const _log_stream& other) = delete;

        _log_stream(_log_stream&& other) noexcept;
        _log_stream& operator=(_log_stream&& other) noexcept;

        ~_log_stream() override;
    };

    // ReSharper enable CppInconsistentNaming

} // namespace nova::renderer

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NOVA_LOG(LEVEL) ::nova::renderer::Logger::instance.log(::nova::renderer::LogLevel::LEVEL)
