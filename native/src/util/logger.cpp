/*!
 * \author ddubois 
 * \date 21-Aug-18.
 */

#include "logger.hpp"

namespace nova {
    logger logger::instance;

    void logger::add_log_handler(log_level level, const std::function<void(std::string)> &log_handler) {
        log_handlers.emplace(level, log_handler);
    }

    void logger::log(const log_level level, const std::string &msg) const {
        if(log_handlers.at(level)) {
            log_handlers.at(level)(msg);
        }
    }

    explicit __log_stream::__log_stream(logger logger, log_level level) : _logger(std::move(logger)), level(level) {}

    __log_stream::~__log_stream() {
        _logger.log(level, str());
    }

    __log_stream logger::log(log_level level) const {
        return __log_stream(*this, level);
    }
}
