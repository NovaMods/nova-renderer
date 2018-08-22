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

    void logger::log(const log_level level, const std::string &msg) {
        if(is_level_enabled.at(level)) {
            log_handlers.at(level)(msg);
        }
    }

    void logger::set_maximum_level_enabled(const log_level level, bool is_enabled) {
        for(log_level i = log_level::TRACE; i < log_level::MAX_LEVEL; i++) {
            if(i < level) {
                is_level_enabled.emplace(i, false);

            } else {
                is_level_enabled.emplace(i, true);
            }
        }
    }
}
