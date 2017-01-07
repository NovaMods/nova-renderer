/*!
 * \brief Defines a bunch of function and structs to help with loading shaders
 *
 * \author ddubois 
 * \date 04-Sep-16.
 */

#ifndef RENDERER_SHADER_LOADING_H_H
#define RENDERER_SHADER_LOADING_H_H

#include <string>
#include <vector>
#include <unordered_map>
#include <utils/export.h>
#include <view/objects/shaders/shaderpack.h>

#include "shader_source_structs.h"

namespace nova {
    /*!
     * \brief Loads the source file of all the shaders with the provided names
     *
     * This will only work if the shaderpack is a zip file. If the shaderpack is just a folder, this will probably
     * fail in strange ways
     *
     * \param shaderpack_name The name of the shaderpack to load the shaders from
     * \param shader_names The list of names of shaders to load
     * \return A map from shader name to shader source
     */
    shaderpack
    load_sources_from_zip_file(const std::string &shaderpack_name, const std::vector<std::string> &shader_names);

    /*!
     * \brief Loads the source file of all the shaders with the provided names
     *
     * This will only work if the shaderpack is a folder. If the shaderpack is a zip folder, this will probably fail
     * in a way I didn't explicitly anticipate
     *
     * \param shaderpack_name The name of the shaderpack to load the shaders from
     * \param shader_names The list of names of shaders to load
     * \return A map from shader name to shader source
     */
    NOVA_API shaderpack
    load_sources_from_folder(const std::string &shaderpack_name, const std::vector<std::string> &shader_names);

    /*!
     * \brief Tries to load a single shader file from a folder
     *
     * Tries appending each string in extensions to the shader path. If one of the extensions is a real extension
     * of the file, returns the full text of the file. If the file cannot be found with any of the provided
     * extensions, then a not_found is thrown
     *
     * \param shader_path The path to the shader
     * \param extensions A list of extensions to try
     * \return The full source of the shader file
     */
    NOVA_API std::vector<shader_line>
    load_shader_file(const std::string &shader_path, const std::vector<std::string> &extensions);

    /*!
     * \brief Loads the shader file from the provided istream
     *
     * \param stream The istream to load the shader file from
     * \param shader_path The path to the shader file (useful mostly for includes)
     * \return A list of shader_line objects
     */
    NOVA_API std::vector<shader_line> read_shader_stream(std::istream &stream, const std::string &shader_path);

    NOVA_API std::vector<shader_line> load_included_file(const std::string &shader_path, const std::string &line);

    auto get_included_file_path(const std::string &shader_path, const std::string &included_file_name);

    NOVA_API std::string get_filename_from_include(const std::string include_line);
}

#endif //RENDERER_SHADER_LOADING_H_H
