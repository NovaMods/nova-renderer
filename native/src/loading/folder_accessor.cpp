/*!
 * \author ddubois 
 * \date 14-Aug-18.
 */

#include "folder_accessor.hpp"
#include "../util/utils.hpp"

namespace nova {
    folder_accessor_base::folder_accessor_base(const fs::path &folder) : our_folder(folder) {}

    bool folder_accessor_base::does_resource_exist(const fs::path & resource_path) {
        auto full_path = our_folder / resource_path;
        return does_resource_exist_internal(full_path);
    }

    std::optional<bool> folder_accessor_base::does_resource_exist_in_map(const std::string &resource_string) const {
        if(resource_existance.find(resource_string) != resource_existance.end()) {
            return std::make_optional<bool>(resource_existance.at(resource_string));
        }

        return {};
    }

    resource_not_found_error::resource_not_found_error(const std::string &resource_name) : runtime_error(resource_name) {}
}
