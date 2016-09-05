/*!
 * \brief Impliments the functions in shader_loading.h
 *
 * \author ddubois 
 * \date 03-Sep-16.
 */

#include <miniz_zip.h>

#include "utils/utils.h"
#include "loaders.h"
#include "shader_loading.h"

namespace nova {
    namespace model {
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

        auto load_sources_from_folder(const std::string& shaderpack_name, const std::vector<const std::string>& shader_names) {
            std::unordered_map<std::string, shader_source> sources;

            for(auto name : shader_names) {
                try {
                    auto shader_path = shaderpack_name + "/shaders/" + name;

                    shader_source full_shader_source = {
                            {load_shader_file(shader_path, vertex_extensions)},
                            {load_shader_file(shader_path, fragment_extensions)}
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
        auto get_file_path(const std::string file_path) {
            auto slash_pos = file_path.find_last_of('/');
            return file_path.substr(0, slash_pos + 1);
        }

        /*!
         * \brief Gets the name of the shaderpack from the file path
         *
         * \param file_path The file path to get the shaderpack name from
         * \return The name of the shaderpack
         */
        auto get_shaderpack_name(const std::string& file_path) {
            auto slash_pos = file_path.find('/');
            return file_path.substr(0, slash_pos);
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
        auto get_filename_from_include(const std::string include_line) {
            auto quote_pos = include_line.find('"');
            return include_line.substr(quote_pos + 1, include_line.size() - quote_pos);
        }

        auto get_included_file_path(const std::string& shader_path, const std::string& included_file_name) {
            if(included_file_name[0] == '/') {
                // This is an absolute include and it should be relative to the root directory
                auto shaderpack_name = get_shaderpack_name(shader_path);
                return shaderpack_name + "/shaders" + included_file_name;

            } else {
                // The include file is a relative include, this one's actually simpler
                auto folder_name = get_file_path(shader_path);
                return folder_name + included_file_name;
            }
        }

        auto load_included_file(const std::string& shader_path, const std::string& line) {
            auto included_file_name = get_filename_from_include(line);
            auto file_to_include = get_included_file_path(shader_path, included_file_name);

            return load_shader_file(file_to_include, {""});
        }

        auto load_shader_file(std::istream& stream, const std::string& shader_path) {
            std::vector<shader_line> file_source;
            std::string line;
            auto line_counter = 0;
            while(std::getline(stream, line, '\n')) {
                if(line.find("#include") == 0) {
                    auto included_file = load_included_file(shader_path, line);
                    file_source.insert(file_source.end(), included_file);

                } else {
                    file_source.push_back({line_counter, shader_path, line});
                }

                line_counter++;
            }

            return file_source;
        }

        auto load_shader_file(const std::string& shader_path, const std::vector<const std::string>& extensions) {
            for(auto extension : extensions) {
                auto full_shader_path = shader_path + extension;

                std::ifstream stream(full_shader_path, std::ios::in);
                if(stream.good()) {
                    return load_shader_file(stream, shader_path);
                }
            }

            throw not_found(shader_path);
        }

        gl_shader_program parse_shader(const shader_source& shader_sources) {
            // The goal here is to go from a bunch of shader source code to an OpenGL shader object, along with
            // information about the uniforms and attributes that the shader has (probably) (I'm honestly not sure how
            // usefun uniforms will be to me)
            //
            // The hard part here is that We have to determine if the shader is a GLSL 120, GLSL 450, or SPIR-V file,
            // and use the appropriate functions to handle each
            // We'll also have to handle #include files, and any other steps that we need to take to process the shader
            // Oh, what joy!

            auto included_source = process_includes(shader_sources);
            //auto compilable_sources = convert_120_to_450(included_source);
        }

        shader_source process_includes(const shader_source& source) {
            auto new_shader_source = shader_source{};
            new_shader_source.vertex_source = process_includes(source);

        }
    }
}
