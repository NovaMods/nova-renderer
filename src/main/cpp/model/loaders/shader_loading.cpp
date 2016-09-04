/*!
 * \brief Impliments the functions in shader_loading.h
 *
 * \author ddubois 
 * \date 03-Sep-16.
 */

#include <vector>

#include <miniz_zip.h>

#include "utils/utils.h"
#include "loaders.h"

namespace nova {
    namespace model {
        /*!
         * \brief Parses the given string into a shader program object
         *
         * \param shader_source The full source code of the shader
         * \return The shader program, in an easy-to-use format
         */
        gl_shader_program parse_shader(const shader_source& shader_sources);

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
                const std::vector<const std::string>& shader_names
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
                const std::vector<const std::string>& shader_names
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
        std::vector<std::string> load_shader_file(
                const std::string shaderpack_name,
                const std::string shader_path,
                const std::vector<const std::string>& extensions
        );

        /*!
         * \brief Checks if the given file is a zip file or not
         *
         * \param filename The name of the file to check
         * \return True if the file is a zipfile, false otherwise
         */
        bool is_zip_file(std::string& filename);

        struct shader_source {
            std::vector<std::string> vertex_source;
            std::vector<std::string> fragment_source;
            // TODO: Figure out how to handle geometry and tessellation shaders
        };

        /*!
         * \brief Holds the name of all the shaders to load
         *
         * As we add more shaders, we should add their names to this vector
         */
        std::vector<const std::string> shader_names = {
                "gui",
        };

        std::vector<const std::string> fragment_extensions = {
                ".fsh",
                ".frag",
                ".frag.spv"
        };

        std::vector<const std::string> vertex_extensions = {
                ".vsh",
                ".vert",
                ".vert.spv"
        };

        std::unordered_map<std::string, gl_shader_program> load_shaderpack(const std::string& shaderpack_name) {
            std::unordered_map<std::string, gl_shader_program> shaderpack;

            auto shader_sources = std::unordered_map<std::string, shader_source>();
            if(is_zip_file(shaderpack_name)) {
                shader_sources = load_sources_from_zip_file(shaderpack_name, shader_names);
            } else {
                shader_sources = load_sources_from_folder(shaderpack_name, shader_names);
            }

            foreach(shader_sources, [&](auto item) {shaderpack.emplace(item.first, parse_shader(item.second));} );

            return shaderpack;
        }

        bool is_zip_file(std::string filename) {
            mz_zip_archive dummy_zip_archive = {};

            return (bool) mz_zip_reader_init_file(&dummy_zip_archive, filename.c_str(), 0, 0, 0);
        }

        std::unordered_map<std::string, shader_source> load_sources_from_folder(
                const std::string& shaderpack_name,
                const std::vector<const std::string>& shader_names
        ) {
            std::unordered_map<std::string, shader_source> sources;

            for(auto name : shader_names) {
                try {
                    shader_source full_shader_source = {
                            load_shader_file(shaderpack_name, name, vertex_extensions),
                            load_shader_file(shaderpack_name, name, fragment_extensions)
                    };

                    sources.emplace(name, full_shader_source);
                } catch(not_found e) {
                    LOG(ERROR) << e.what();
                }
            }

            return sources;
        }

        /*!
         * \brief Lops off everything after the first '/' in the file path, returning only the bit before it with the
         * slash included
         *
         * \param file_path The file path to lop the name off of
         * \return The path to the folder that the provided file resides in
         */
        std::string lop_off_file_name(const std::string file_path) {
            auto slash_pos = file_path.find_last_of('/');
            return file_path.substr(0, slash_pos + 1);
        }

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
        std::string get_filename_from_include(const std::string include_line) {
            auto quote_pos = include_line.find('"');
            return include_line.substr(quote_pos + 1, include_line.size() - 1);
        }

        std::vector<std::string> load_shader_file(
                const std::string shaderpack_name,
                const std::string shader_name,
                const std::vector<const std::string>& extensions
        ) {
            for(auto extension : extensions) {
                auto shader_path_base = shaderpack_name + "/shaders/" + shader_name;
                auto filename = shader_path_base + extension;
                std::vector<std::string> file_source;

                try {
                    std::stringstream stream(read_file(filename));
                    std::string line;
                    while(std::getline(stream, line, '\n')) {
                        if(line.find("#include") != std::string::npos) {
                            auto included_filename = get_filename_from_include(line);
                            LOG(DEBUG) << "Found included file " << included_filename << " in shader file " << filename;

                            if(included_filename[0] == '/') {
                                // The path to this file is relative to the shaders folder
                                included_filename = "";
                            }

                            auto base_path = lop_off_file_name(shader_name);

                            // The #include statement should have the file extension in it, so
                            auto included_file = load_shader_file(shaderpack_name, included_filename, {""});
                            foreach(included_file, file_source.push_back);
                        } else {
                            file_source.push_back(line);
                        }
                    }

                    return file_source;

                } catch(int err) {
                    // If we get an exception, we want to try to load the shader file with the next extension
                    // The exception really isn't a problem, so we can honestly just ignore it
                }
            }

            throw not_found(shader_name);
        }

        gl_shader_program parse_shader(const shader_source& shader_sources) {
            // The goal here is to go from a bunch of shader source code to an OpenGL shader object, along with
            // information about the uniforms and attributes that the shader has (probably) (I'm honestly not sure how
            // usefun uniforms will be to me)
            //
            // The hard part here is that We have to determine if the shader is a GLSL 120, GLSL 450, or SPIR-V file,
            // and use the appropriate functions to handle each
            // Oh, what joy!

        }
    }
}
