/*!
 * \author ddubois
 * \date 21-Aug-18.
 */

#include "logger.hpp"

namespace nova {
    logger logger::instance;

    void logger::add_log_handler(log_level level, const std::function<void(std::string)>& log_handler) {
        log_handlers.emplace(level, log_handler);
    }

    void logger::log(const log_level level, const std::string& msg) {
        if(log_handlers.at(level)) {
            std::lock_guard<std::mutex> lock(log_lock);
            log_handlers.at(level)(msg);
        }
    }

    _log_stream::_log_stream(log_level level) : level(level) {}

    _log_stream::_log_stream(_log_stream&& other) noexcept : std::stringstream(std::move(other)), level(other.level) {}

    _log_stream& _log_stream::operator=(_log_stream&& other) noexcept {
        level = other.level;
        std::stringstream::operator=(std::move(other));
        return *this;
    }

    _log_stream::~_log_stream() { logger::instance.log(level, str()); }

    _log_stream logger::log(log_level level) const { return _log_stream(level); }
} // namespace nova
