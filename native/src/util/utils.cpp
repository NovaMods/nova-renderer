/*!
 * \author David
 * \date 18-May-16.
 */

#include "utils.hpp"
#include "logger.hpp"

namespace nova {
    resource_not_found::resource_not_found(const std::string &msg) {
        message = "Could not find resource " + msg;
    }

    const char * resource_not_found::what() const noexcept {
        return message.c_str();
    }

    nlohmann::json load_json_from_stream(std::istream& stream) {
        if(stream.bad()) {
            logger::instance::log(log_level::ERROR, std::string("Bad stream~ EOF? ") + (stream.eof() ? "true" : "false") + " If not, it must be something else. Returning empty JSON cause life is hard");
            return {};
        }

        std::string buf;
        std::string accum;

        while(getline(stream, buf)) {
            bool line_is_comment = false;

            // Ensure that the line isn't a comment
            // Gonna discard all the lines with // as the first non-whitespace character
            auto comment_pos = buf.find("//");
            if(comment_pos != std::string::npos) {
                line_is_comment = true;
                logger::instance::log(log_level::TRACE, "Found possible comment line `" + buf + "`");
                for(size_t i = 0; i < comment_pos; i++) {
                    if(buf[i] != ' ' && buf[i] != '\t' && buf[i] != '\n') {
                        line_is_comment = false;
                        logger::instance::log(log_level::TRACE, "There's stuff in front of it so it hopefully isn't a comment");
                    }
                }
                if(line_is_comment) {
                    logger::instance::log(log_level::DEBUG, "Skipping comment line `" + buf + "`");
                }
            }

            if(!line_is_comment) {
                accum += buf;
            }
        }

        try {
            return nlohmann::json::parse(accum.c_str());
        } catch(std::exception& e) {
            logger::instance::log(log_level::ERROR, e.what());
            return {};
        }
    }

    std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return elems;
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

    std::string print_array(int data[], int size) {
        std::stringstream ss;

        for(int i = 0; i < size; i++) {
            ss << data[i] << " ";
        }

        return ss.str();
    }
}
