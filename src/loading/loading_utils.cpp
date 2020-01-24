<<<<<<< HEAD
#include "loading_utils.hpp"

namespace nova::renderer {
    bool is_zip_folder(const fs::path& path_to_folder) {
        auto extension = path_to_folder.extension();
        return path_to_folder.has_extension() && extension == ".zip";
    }
=======
#pragma once

#include <rx/core/string.h>

namespace nova::renderer {
>>>>>>> [loading] Moved a bunch of files so I don't have to deal with them just yet
} // namespace nova::renderer
