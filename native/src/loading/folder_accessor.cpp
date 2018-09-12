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

    bool has_root(const fs::path& path, const fs::path& root) {
        if(std::distance(path.begin(), path.end()) < std::distance(root.begin(), root.end())) {
            // The path is shorter than the root path - the root can't possible be contained in the path
            return false;
        }

        auto path_itr = path.begin();
        auto root_itr = root.begin();
        while(root_itr != root.end()) {
            // Don't need to check path_itr - The if statement at the beginning ensures that the path has more members
            // than the root
            if(*root_itr != *path_itr) {
                return false;
            }

            ++root_itr;
            ++path_itr;
        }

        return true;
    }

    resource_not_found_error::resource_not_found_error(const std::string &resource_name) : runtime_error(resource_name) {}
}
