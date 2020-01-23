#pragma once

#include <rx/core/string.h>

namespace nova::renderer {
    /*!
     * \brief Determines if a given path refers to a zip folder or a regular folder
     *
     * \param path_to_folder The path from Nova's working directory to the folder you want to check
     *
     * \return True if the folder in question is a zip folder, false otherwise
     */
    inline bool is_zip_folder(const rx::string& path_to_folder);

    bool is_zip_folder(const rx::string& path_to_folder) { return path_to_folder.ends_with(".zip"); }
} // namespace nova::renderer
