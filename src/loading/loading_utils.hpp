/*!
 * \author ddubois
 * \date 21-Aug-18.
 */

#ifndef NOVA_RENDERER_LOADING_UTILS_HPP
#define NOVA_RENDERER_LOADING_UTILS_HPP

#include <string>

#include "../../include/nova_renderer/util/filesystem.hpp"

namespace nova::renderer {
    /*!
     * \brief Determines if a given path refers to a zip folder or a regular folder
     * \param path_to_folder The path from Nova's working directory to the folder you want to check
     * \return True if the folder in question is a zip folder, false otherwise
     */
    bool is_zip_folder(const fs::path& path_to_folder);
} // namespace nova::renderer

#endif // NOVA_RENDERER_LOADING_UTILS_HPP
