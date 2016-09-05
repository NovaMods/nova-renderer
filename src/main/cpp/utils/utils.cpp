/*!
 * \author David
 * \date 18-May-16.
 */

#include <easylogging++.h>

#include <iostream>

#include "utils.h"

void initialize_logging() {
    // Configure the logger
    el::Configurations conf("config/logging.conf");

    // Turn debug and trace off in release builds
#ifdef NDEBUG
    conf.parseFromText("*DEBUG:\n ENABLED=false");
    conf.parseFromText("*TRACE:\n ENABLED=false");
#else
    conf.parseFromText("*ALL: FORMAT = \"%datetime{%h:%m:%s} [%level] at %loc - %msg\"");
#endif

    el::Loggers::reconfigureAllLoggers(conf);
}

namespace nova {
    resource_not_found::resource_not_found(const std::string &msg) {
        message = "Could not fine resource " + msg;
    }

    const char * resource_not_found::what() const noexcept {
        return message.c_str();
    }
}

