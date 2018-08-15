/*!
 * \author ddubois 
 * \date 14-Aug-18.
 */

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
            throw std::runtime_error("Resouce at path " + resource_path.string() + " does not exist");
        }
    }
}
