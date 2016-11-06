/*!
 * \brief Impliments the functions in shader_loading.h
 *
 * \author ddubois 
 * \date 03-Sep-16.
 */

#include <easylogging++.h>

#include "utils/utils.h"
#include "loaders.h"
#include "shader_loading.h"
#include "loader_utils.h"

namespace nova {
    namespace model {
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

        std::unordered_map<std::string, shader_definition> load_shaderpack(const std::string& shaderpack_name) {
            auto shader_sources = std::unordered_map<std::string, shader_definition>{};
            if(is_zip_file(shaderpack_name)) {
                LOG(TRACE) << "Loading shaderpack " << shaderpack_name << " from a zip file";
                shader_sources = load_sources_from_zip_file(shaderpack_name, shader_names);
            } else {
                LOG(TRACE) << "Loading shaderpack " << shaderpack_name << " from a regular folder";
                shader_sources = load_sources_from_folder(shaderpack_name, shader_names);
            }

            return shader_sources;
        }

        std::unordered_map<std::string, shader_definition> load_sources_from_folder(
                const std::string& shaderpack_name,
                const std::vector<std::string>& shader_names
        ) {
            std::unordered_map<std::string, shader_definition> sources;

            for(auto& name : shader_names) {
                try {
                    // All shaderpacks are in the shaderpacks folder
                    auto shader_path = "shaderpacks/" + shaderpack_name + "/shaders/" + name;

                    shader_definition full_shader_source = {
                            load_shader_file(shader_path, vertex_extensions),
                            load_shader_file(shader_path, fragment_extensions)
                    };

                    sources.emplace(name, full_shader_source);
                } catch(std::exception e) {
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
        std::string get_filename_from_include(const std::string include_line) {
            auto quote_pos = include_line.find('"');
            return include_line.substr(quote_pos + 1, include_line.size() - quote_pos - 2);
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

        std::vector<shader_line> load_shader_file(const std::string& shader_path, const std::vector<std::string>& extensions) {
            for(auto& extension : extensions) {
                auto full_shader_path = shader_path + extension;
                LOG(TRACE) << "Loading shader file " << full_shader_path;

                std::ifstream stream(full_shader_path, std::ios::in);
                if(stream.good()) {
                    return read_shader_stream(stream, full_shader_path);
                } else {
                    LOG(WARNING) << "Could not read file " << full_shader_path;
                }
            }

            throw resource_not_found(shader_path);
        }

        std::vector<shader_line> read_shader_stream(std::istream &stream, const std::string &shader_path) {
            std::vector<shader_line> file_source;
            std::string line;
            auto line_counter = 1;
            while(std::getline(stream, line, '\n')) {
                if(line.find("#include") == 0) {
                    auto included_file = load_included_file(shader_path, line);
                    file_source.insert(file_source.end(), std::begin(included_file), std::end(included_file));

                } else {
                    file_source.push_back({line_counter, shader_path, line});
                }

                line_counter++;
            }

            return file_source;
        }

        std::vector<shader_line> load_included_file(const std::string& shader_path, const std::string& line) {
            auto included_file_name = get_filename_from_include(line);
            auto file_to_include = get_included_file_path(shader_path, included_file_name);
            LOG(TRACE) << "Dealing with included file " << file_to_include;

            return load_shader_file(file_to_include, {""});
        }

        std::unordered_map<std::string, shader_definition> load_sources_from_zip_file(
                const std::string& shaderpack_name,
                const std::vector<std::string>& shader_names
        ) {
            LOG(FATAL) << "Cannot load zipped shaderpack " << shaderpack_name;
            return {};
        }
    }
}
