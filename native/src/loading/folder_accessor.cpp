/*!
 * \author ddubois 
 * \date 14-Aug-18.
 */

#include "folder_accessor.hpp"
#include "../util/utils.hpp"

namespace nova {
    folder_accessor_base::folder_accessor_base(const fs::path &folder) : our_folder(folder) {}

    std::optional<bool> folder_accessor_base::does_resource_exist_in_map(const std::string &resource_string) const {
        if(resource_existance.find(resource_string) != resource_existance.end()) {
            return std::make_optional<bool>(resource_existance.at(resource_string));
        }

        return {};
    }

    bool folder_accessor_base::is_path_relative_to_our_root(const fs::path & resource_path) {
        std::vector<std::string> root_path_split = split(our_folder.string(), '/');
        std::vector<std::string> resource_path_split = split(resource_path.string(), '/');

        bool has_root = true;
        for(uint32_t i = 0; i < root_path_split.size() && i < resource_path_split.size(); i++) {
            if(root_path_split[i] != resource_path_split[i]) {
                has_root = false;
                break;
            }
        }

        return has_root;
    }

    fs::path folder_accessor_base::get_full_path(const fs::path path) {
        bool has_root = is_path_relative_to_our_root(path);

        fs::path full_resource_path;
        if(has_root) {
            full_resource_path = resource_path;

        } else {
            full_resource_path = our_folder / resource_path;
        }

        return full_resource_path;
    }

    resource_not_found_error::resource_not_found_error(const std::string &resource_name) : runtime_error(resource_name) {}
}
