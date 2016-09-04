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
    std::string read_file(const std::string& filename) {
        std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
        if(in) {
            std::string contents;
            in.seekg(0, std::ios::end);
            contents.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&contents[0], contents.size());
            in.close();
            return (contents);
        }
        throw (errno);
    }

    not_found::not_found(const std::string &msg) {
        message = "Could not fine resource " + msg;
    }

    const char * not_found::what() const noexcept {
        return message.c_str();
    }
}

