/*!
 * \brief Defines a bunch of function and structs to help with loading shaders
 *
 * \author ddubois 
 * \date 04-Sep-16.
 */

#ifndef RENDERER_SHADER_LOADING_H_H
#define RENDERER_SHADER_LOADING_H_H

#include <experimental/filesystem>
#include <string>
#include <vector>
#include <unordered_map>

#include "shader_source_structs.h"
#include "../../render/objects/renderpasses/materials.h"
#include "../../render/objects/renderpasses/render_passes.h"

namespace fs = std::experimental::filesystem;

namespace nova {
    /*!
     * \brief A shaderpack loaded from disk
     *
     * A shaderpack has
     * - A list of render passes. Render passes have:
     *      - The dynamic resources that this pass needs as an input
     *      - The dynamic resources that this pass writes to
     * - A list of pipelines. A pipeline has
     *      - A fragment shader and vertex shader, and optionally a geometry shader and tessellation shader
     *      - All sorts of renderer state like depth test, stencil test, etc
     *      - The textures and UBOs the shader uses, and what location to bind them to
     *      - The textures in the framebuffer that this shader writes to, and where to bind them
     *      - The name of the pass that should render this pipeline
     *      - A filter that selects geometry that this shader renders
     * - A list of dynamic resources. A dynamic resource is a texture or buffer that is written to by a shader. Nova 1
     *  only supports textures (unless that changes). A dynamic texture has:
     *      - A name
     *      - A resolution (relative to the screen, or in absolute pixels)
     *      - A format
     */
    struct shaderpack_data {
        std::unordered_map<std::string, std::vector<pipeline>> pipelines_by_pass;
        std::unordered_map<std::string, render_pass> passes;
        std::unordered_map<std::string, texture_resource> dynamic_textures;
        std::unordered_map<std::string, std::vector<material>> materials;
    };

    /*!
     * \brief Loads all the passes that are present in the shaderpack with the given zip file name
     *
     * \param shaderpack_name The name of the shaderpack file to load things from
     * \return A map from pass name to pass of all the passes found in that zip
     */
    std::unordered_map<std::string, pipeline> load_passes_from_zip(const std::string& shaderpack_name);

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
    std::unordered_map<std::string, shader_definition> load_sources_from_zip_file(const std::string &shaderpack_name, const std::vector<std::string> &shader_names);

    /*!
     * \brief Loads all the pipelines that are present in the shaderpack with the given folder name
     *
     * \param shaderpack_path The name of the shaderpack folder to load things from
     * \return A map from pipeline name to list of all the pipelines found in that folder
     */
    std::unordered_map<std::string, std::vector<pipeline>> load_pipelines_from_folder(const fs::path &shaderpack_path);

    /*!
     * \brief Loads all the passes that were defined in a given shaderpack's folder
     *
     * \param shaderpack_path The path to the shaderpack's folder
     * \return A map from the name of the pass to that pass
     */
    std::unordered_map<std::string, render_pass> load_passes_from_folder(const fs::path& shaderpack_path);

    std::unordered_map<std::string, texture_resource> load_texture_definitions_from_folder(const fs::path& shaderpack_path);

    std::unordered_map<std::string, std::vector<material>> load_materials_from_folder(const fs::path& shaderpack_path);

    std::unordered_map<std::string, render_pass> parse_passes_from_json(const nlohmann::json& json);

    std::vector<pipeline> read_pipeline_files(const fs::path& shaderpack_path);

    std::vector<pipeline> parse_pipelines_from_json(const nlohmann::json &pipelines_json);

    /*!
     * \brief Parses a list of texture resource definitions from the provided JSON array
     * \param json A JSON array of texture_resoruce objects
     * \return A map from the name of the texture resource to the texture resoruce itself
     */
    std::unordered_map<std::string, texture_resource> parse_textures_from_json(nlohmann::json& json);

    /*!
     * \brief Gets a list of all the files in the given folder
     *
     * \param shaderpack_name The name of the shaderpack to get the names of the shaders in
     * \return The names of all the shaders in
     */
    std::vector<fs::path> get_shader_names_in_folder(const fs::path& shaderpack_path);

    /*!
     * \brief Loads the source file of all the shaders with the provided names
     *
     * This will only work if the shaderpack is a folder. If the shaderpack is a zip folder, this will probably fail
     * in a way I didn't explicitly anticipate
     *
     * \param shaders_path The name of the shaderpack to load the shaders from
     * \param pipelines The list of names of shaders to load
     * \return The loaded shaderpack
     */
    std::unordered_map<std::string, shader_definition> load_sources_from_folder(const fs::path &shaders_path, const std::vector<pipeline> &pipelines);

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
    std::string get_filename_from_include(const std::string& include_line);

    /*!
     * \brief Loads the JSON for the default Bedrock pipeline files
     *
     * \return The JSON for the default Bedrock pipeline files
     */
    nlohmann::json& get_default_bedrock_passes();

    /*!
     * \brief Loads the JSON for the default Optifine passes
     *
     * \return The JSON for the default Optifine passes
     */
    nlohmann::json& get_default_optifine_passes();
}

#endif //RENDERER_SHADER_LOADING_H_H
