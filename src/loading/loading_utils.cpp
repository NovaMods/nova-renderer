/*!
 * \author ddubois
 * \date 21-Aug-18.
 */

#include "loading_utils.hpp"

namespace nova {
    bool is_zip_folder(const fs::path& path_to_folder) {
        auto extension = path_to_folder.extension();
        return path_to_folder.has_extension() && extension == ".zip";
    }
} // namespace nova
