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
        log_handlers.at(level)(msg);
    }
}
