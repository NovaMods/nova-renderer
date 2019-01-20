/*!
 * \author David
 * \date 18-May-16.
 */

#include <cstring>
#include <utility>
#include "utils.hpp"
#include "logger.hpp"

namespace nova {
    // taken from https://www.fluentcpp.com/2017/04/21/how-to-split-a-string-in-c/
    std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while(std::getline(tokenStream, token, delim)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    std::string join(const std::vector<std::string> &strings, const std::string &joiner = ", ") {
        std::stringstream ss;
        for(size_t i = 0; i < strings.size(); i++) {
            ss << strings[i];
            if(i < strings.size() - 1) {
                ss << joiner;
            }
        }

        return ss.str();
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

    bool ends_with(const std::string &string, const std::string &ending) {
        if(string.length() >= ending.length()) {
            return (0 == string.compare(string.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }

    void write_to_file(const std::string& data, const fs::path& filepath) {
        std::ofstream os(filepath);
        if(os.good()) {
            os << data;
        }
        os.close();
    }

    void write_to_file(const std::vector<uint32_t>& data, const fs::path& filepath) {
        std::ofstream os(filepath, std::ios::binary);
        if(os.good()) {
            os.write(reinterpret_cast<const char*>(data.data()), data.size() * 4);
        }
        os.close();
    }

    nova_exception::nova_exception() : msg(typeid(*this).name()), cause(std::runtime_error("<UNINITIALIZED>")) {}

    nova_exception::nova_exception(const std::exception& cause) : cause(cause) {}

    nova_exception::nova_exception(std::string msg) : msg(std::move(msg)), cause(std::runtime_error("<UNINITIALIZED>")) {}

    nova_exception::nova_exception(std::string msg, const std::exception& cause) : msg(std::move(msg)), cause(cause) {}

    const char *nova_exception::what() const noexcept {
		std::stringstream ss;

		ss << msg;

        if(std::strcmp(cause.what(), "<UNINITIALIZED>") != 0) {
			ss << "\nCaused by: " << cause.what();
        }

        return ss.str().c_str();
    }
}  // namespace nova
