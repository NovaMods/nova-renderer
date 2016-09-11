/*!
 * \brief
 *
 * \author ddubois 
 * \date 05-Sep-16.
 */

#define MINIZ_HEADER_FILE_ONLY
#include <3rdparty/miniz/src/miniz.c>

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include <3rdparty/stb_image/stb_image.h>
#include "loaders.h"
#include "texture_loading.h"
#include "loader_utils.h"

namespace nova {
    namespace model{
        std::unordered_map<std::string, GLuint> load_textures(const std::vector<std::string>& resourcepack_names) {
            std::unordered_map<std::string, texture_data> loaded_textures;

            for(const auto& pack_name : resourcepack_names) {
                auto textures = load_textures_from_resourcepack(pack_name);
                loaded_textures.insert(std::begin(textures), std::end(textures));
            }

            // We have all the textures. Let's pack them into a texture atlas

            return {};
        };

        std::unordered_map<std::string, texture_data> load_textures_from_resourcepack(const std::string& resourcepack_name) {
            // First check if the resourcepack is a zip file or a regular folder
            // Nova supports both, but we're only supporting zip resourcepacks for the time being

            // TODO: Support folder-based resourcepacks
            if(is_zip_file(resourcepack_name)) {
                return load_textures_zip(resourcepack_name);
            } else {
                // TODO:
                return {};
            }
        };

        std::unordered_map<std::string, texture_data> load_textures_zip(const std::string& resourcepack_name) {
            mz_zip_archive zip_archive = {};
            if(!mz_zip_reader_init_file(&zip_archive, resourcepack_name.c_str(), 0)) {
                LOG(ERROR) << "Could not open resourcepack " << resourcepack_name;
            }

            std::unordered_map<std::string, texture_data> loaded_textures;

            for(auto& texture_name : textures_to_load) {
                int file_loc = mz_zip_reader_locate_file(&zip_archive, texture_name.c_str(), nullptr, 0);
                if(file_loc == -1) {
                    LOG(TRACE) << "Could not file file " << texture_name << " in resourcepack " << resourcepack_name;
                    continue;
                }

                mz_zip_archive_file_stat file_stat = {};

                if(mz_zip_reader_file_stat(&zip_archive, (mz_uint) file_loc, &file_stat)) {
                    unsigned char file_buf[file_stat.m_uncomp_size];

                    if(mz_zip_reader_extract_to_mem(&zip_archive, (mz_uint) file_loc, file_buf, file_stat.m_uncomp_size, 0)) {
                        // File extraction successful!
                        texture_data tex_data = {};
                        tex_data.data = stbi_load_from_memory(file_buf, (int) file_stat.m_uncomp_size, &tex_data.width, &tex_data.height, &tex_data.num_components, 0);
                        loaded_textures.emplace(texture_name, tex_data);

                    } else {
                        LOG(DEBUG) << "Could not extract file " << texture_name << " from archive " << resourcepack_name;
                    }

                } else {
                    LOG(DEBUG) << "Could not get file stats on file at index " << file_loc;
                }
            }

            mz_zip_reader_end(&zip_archive);

            return {};
        }
    };
}
