/*!
 * \author David
 * \date 18-May-16.
 */

#include <easylogging++.h>

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
        message = "Could not find resource " + msg;
    }

    const char * resource_not_found::what() const noexcept {
        return message.c_str();
    }

    nlohmann::json load_json_from_stream(std::istream& stream) {
        std::string buf;
        std::string accum;

        while(getline(stream, buf)) {
            accum += buf;
        }

        return nlohmann::json::parse(accum.c_str());
    }

    std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return elems;
    }

    el::base::Writer &operator<<(el::base::Writer &out, const glm::ivec3 &vec) {
        out << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return out;
    }

    el::base::Writer &operator<<(el::base::Writer &out, const glm::vec2 &vec) {
        out << "(" << vec.x << ", " << vec.y << ")";
        return out;
    }

    el::base::Writer &operator<<(el::base::Writer &out, const glm::vec3 &vec) {
        out << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return out;
    }

    el::base::Writer &operator<<(el::base::Writer &out, const glm::vec4 &vec) {
        out << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
        return out;
    }

    std::string print_color(unsigned int color) {
        auto red = color >> 24;
        auto green = (color >> 16) & 0xFF;
        auto blue = (color >> 8) & 0xFF;
        auto alpha = color & 0xFF;

        std::stringstream str;
        str << "(" << red << ", " << green << ", " << blue << ", " << alpha << ")";

        return str.str();
    }
}

