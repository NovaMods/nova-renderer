/*!
 * \author ddubois 
 * \date 21-Aug-18.
 */

#include "loading_utils.hpp"

namespace nova {
    bool is_zip_folder(const fs::path& path_to_folder) {
        fs::path zip_path = path_to_folder;
        zip_path.replace_extension(".zip");
        return !fs::exists(zip_path);
    }
}
