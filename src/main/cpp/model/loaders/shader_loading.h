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

#include "model/gl/gl_shader_program.h"

namespace nova {
    namespace model {
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
        std::unordered_map<std::string, shader_source> load_sources_from_zip_file(
                const std::string& shaderpack_name,
                const std::vector<std::string>& shader_names
        );

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
        std::unordered_map<std::string, shader_source> load_sources_from_folder(
                const std::string& shaderpack_name,
                const std::vector<std::string>& shader_names
        );

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
        std::vector<shader_line> load_shader_file(const std::string& shader_path, const std::vector<std::string>& extensions);

        /*!
         * \brief Loads the shader file from the provided istream
         *
         * \param stream The istream to load the shader file from
         * \param shader_path The path to the shader file (useful mostly for includes)
         * \return A list of shader_line objects
         */
        std::vector<shader_line> read_shader_stream(std::istream &stream, const std::string &shader_path);

        std::vector<shader_line> load_included_file(const std::string& shader_path, const std::string& line);

        /*!
         * \brief Holds the name of all the shaders to load
         *
         * As we add more shaders, we should add their names to this vector
         */
        std::vector<std::string> shader_names = {
                "gui"
        };

        std::vector<std::string> fragment_extensions = {
                ".fsh",
                ".frag",
                ".frag.spv"
        };

        std::vector<std::string> vertex_extensions = {
                ".vsh",
                ".vert",
                ".vert.spv"
        };
    }
}

#endif //RENDERER_SHADER_LOADING_H_H
