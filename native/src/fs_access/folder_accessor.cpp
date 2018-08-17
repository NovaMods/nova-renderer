/*!
 * \author ddubois 
 * \date 14-Aug-18.
 */

#include "folder_accessor.h"

namespace nova {
    folder_accessor_base::folder_accessor_base(const fs::path &folder) : our_folder(folder) {}

    std::optional<bool> folder_accessor_base::does_resource_exist_in_map(const std::string &resource_string) const {
        if(resource_existance.find(resource_string) != resource_existance.end()) {
            return std::make_optional<bool>(resource_existance.at(resource_string));
        }

        return {};
    }

    resource_not_found_error::resource_not_found_error(const std::string &resource_name) : runtime_error(resource_name) {}
}
