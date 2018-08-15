/*!
 * \author ddubois 
 * \date 14-Aug-18.
 */

#include <fstream>
#include "regular_folder_accessor.h"

namespace nova {
    regular_folder_accessor::regular_folder_accessor(const std::experimental::filesystem::path &folder)
            : folder_accessor_base(folder) {}

    bool regular_folder_accessor::does_resource_exist(const fs::path &resource_path) {
        const auto resource_string = resource_path.string();
        const auto existance_maybe = does_resource_exist_in_map(resource_string);
        if(existance_maybe) {
            return existance_maybe.value();
        }

        if(fs::exists(resource_path)) {
            resource_existance.emplace(resource_string, true);
            return true;

        } else {
            resource_existance.emplace(resource_string, false);
            return false;
        }
    }

    std::vector<uint8_t> regular_folder_accessor::load_resource(const fs::path &resource_path) {
        if(!does_resource_exist(resource_path)) {
            throw std::runtime_error("Resource at path " + resource_path.string() + " does not exist");
        }

        std::vector<uint8_t> buf;
        std::ifstream resource_stream(resource_path.string());
        if(!resource_stream.good()) {
            // Error reading this file - it can't be read again in the future
            const auto resource_string = resource_path.string();

            resource_existance.emplace(resource_string, false);
            throw std::runtime_error("Could not load resource at path " + resource_string);
        }

        while(!resource_stream.eof()) {
            uint8_t val;
            resource_stream >> val;
            buf.push_back(val);
        }

        return buf;
    }
}
