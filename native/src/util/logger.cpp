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
        log_handlers.at(level)(msg);
    }

    __log_stream logger::log(log_level level) const {
        return __log_stream(*this, level);
    }
}
