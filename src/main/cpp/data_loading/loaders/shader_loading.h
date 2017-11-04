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
#include <shaderc/shaderc.h>

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
    std::vector<shader_definition> load_sources_from_zip_file(const std::string &shaderpack_name, std::vector<std::string> shader_names);

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
    std::vector<shader_definition> load_sources_from_folder(const std::string &shaderpack_name, std::vector<std::string>& shader_names);

    /*!
     * \brief Tries to load a single shader file from a folder
     *
     * Tries appending each string in extensions to the shader path. If one of the extensions is a real extension
     * of the file, returns the full text of the file. If the file cannot be found with any of the provided
     * extensions, then a not_found is thrown
     *
     * This function also compiled the shader to SPIR-V if needed and hopefully throw reasonable errors
     *
     * \param shader_path The path to the shader
     * \param extensions A list of extensions to try
     * \return The SPIR-V of the shader file
     */
    std::vector<shader_line> load_shader_file(std::string shader_path, std::vector<std::string> extensions);

    /*!
     * \brief Translates GLSL code to SPIR-V
     *
     * \param shader_lines The source lines of the code to translate
     * \param shader_stage The shader stage to compile for
     * \return The SPIR-V code
     */
    std::vector<uint32_t> translate_glsl_to_spirv(std::vector<shader_line> shader_lines, shaderc_shader_kind shader_stage);

    /*!
     * \brief Loads the shader file from the provided istream
     *
     * \param stream The istream to load the shader file from
     * \param shader_path The path to the shader file (useful mostly for includes)
     * \return A list of shader_line objects
     */
    std::vector<shader_line> read_shader_stream(std::istream &stream, const std::string &shader_path);

    /*!
     * \brief Loads a file that was requested through a #include statement
     *
     * This function will recursively include files. There's nothing to check for an infinite include loop, so try to
     * not have any
     *
     * \param shader_path The path to the shader that includes the file
     * \param line The line in the shader that contains the #include statement
     * \return The full source of the included file
     */
    std::vector<shader_line> load_included_file(const std::string &shader_path, const std::string &line);

    /*!
     * \brief Determines the full file path of an included file
     *
     * \param shader_path The path to the current shader
     * \param included_file_name The name of the file to include
     * \return The path to the included file
     */
    auto get_included_file_path(const std::string &shader_path, const std::string &included_file_name);

    /*!
     * \brief Extracts the filename from the #include line
     *
     * #include lines look like
     *
     * #include "shader.glsl"
     *
     * I need to get the bit inside the quotes
     *
     * \param include_line The line with the #include statement on it
     * \return The filename that is being included
     */
    std::string get_filename_from_include(const std::string include_line);

    /*!
     * \brief Figures out the names of the shaders to load into this shaderpack based on the given json structure
     *
     * \param shaders_json The JSON structure with the names of all the shaders to load
     * \return A list of all the shader_definition objects described by the given JSON
     */
    std::vector<shader_definition> get_shader_definitions(nlohmann::json &shaders_json);

    /*!
     * \brief Prints our a warning for every shader described in the shaders.json file which does not specify a
     * fallback shader
     *
     * \param shaders The list of shader definitions from the shaders.json file
     */
    void warn_for_missing_fallbacks(std::vector<shader_definition> shaders);

    /*!
     * \brief Loads the default shader.json file from disk
     *
     * This function caches the shaders.json file, so it should only load once
     *
     * \return The default shader.json file, as a json data structure
     */
    nlohmann::json& get_default_shaders_json();
}

#endif //RENDERER_SHADER_LOADING_H_H
