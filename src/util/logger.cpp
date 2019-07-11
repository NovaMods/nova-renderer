#include "logger.hpp"

namespace nova::renderer {
	Logger Logger::instance;

    void Logger::add_log_handler(LogLevel level, const eastl::function<void(eastl::string)>& log_handler) {
        log_handlers.emplace(level, log_handler);
    }

    void Logger::log(const LogLevel level, const eastl::string& msg) {
        if(log_handlers.at(level)) {
            std::lock_guard<std::mutex> lock(log_lock);
            log_handlers.at(level)(msg);
        }
    }

    _log_stream::_log_stream(LogLevel level) : level(level) {}

    _log_stream::_log_stream(_log_stream&& other) noexcept : std::stringstream(eastl::move(other)), level(other.level) {}

    _log_stream& _log_stream::operator=(_log_stream&& other) noexcept {
        level = other.level;
        std::stringstream::operator=(eastl::move(other));
        return *this;
    }

    _log_stream::~_log_stream() { Logger::instance.log(level, str().c_str()); }

    _log_stream Logger::log(LogLevel level) const { return _log_stream(level); }
} // namespace nova::renderer
