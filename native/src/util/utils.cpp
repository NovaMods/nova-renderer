/*!
 * \author David
 * \date 18-May-16.
 */

#include "utils.hpp"
#include "logger.hpp"

namespace nova {
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
