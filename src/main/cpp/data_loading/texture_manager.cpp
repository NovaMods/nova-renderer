/*!
 * \author David
 * \date 29-Apr-16.
 */

#include <algorithm>
#define ELPP_THREAD_SAFE
#include <easylogging++.h>
#include "texture_manager.h"

namespace nova {
    namespace model {
        texture_manager::texture_manager() {
            LOG(INFO) << "Creating the Texture Manager";
            reset();
            LOG(INFO) << "Texture manager created";
        }

        texture_manager::~texture_manager() {
            // gotta free up all the OpenGL textures
            // The driver probably does that for me, but I ain't about to let no stinkin' driver boss me around!
            reset();
        }

        void texture_manager::reset() {
            if(atlases.empty()) {
                // Nothing to deallocate, let's just return
                return;
            }
            // Gather all the textures into a list so we only need one call to delete them
            std::vector<GLuint> texture_ids(atlases.size());
            for(auto tex : atlases) {
                texture_ids.push_back(tex.second.get_gl_name());
            }

            glDeleteTextures((GLsizei) texture_ids.size(), texture_ids.data());

            atlases.clear();
            locations.clear();
        }

        void texture_manager::add_texture(mc_atlas_texture &new_texture, atlas_type type, texture_type data_type) {
            // TODO:
            // Create an OpenGL texture from the given texture
            // Save it to the list of atlas textures
            LOG(DEBUG) << "Creating a Texture2D for this atlas";
            texture2D texture;

            LOG(DEBUG) << "Converting the pixel data to a float";
            std::vector<float> pixel_data(
                    (size_t) (new_texture.width * new_texture.height * new_texture.num_components));
            for(int i = 0; i < new_texture.width * new_texture.height * new_texture.num_components; i++) {
                pixel_data[i] = float(new_texture.texture_data[i]) / 255.0f;
            }

            LOG(DEBUG) << "Added all pixel data";

            std::vector<int> dimensions = {new_texture.width, new_texture.height};

            GLenum format = GL_RGB;
            switch(new_texture.num_components) {
                case 1:
                    format = GL_RED;
                    break;
                case 2:
                    format = GL_RG;
                    break;
                case 3:
                    format = GL_RGB;
                    break;
                case 4:
                    format = GL_RGBA;
                    break;
                default:
                    LOG(ERROR) << "Unsupported number of components. You have " << new_texture.num_components
                               << " components "
                               << ", but I need a number in [1,4]";
            }

            texture.set_data(pixel_data, dimensions, format);
            LOG(DEBUG) << "Texture data sent to GPU";

            atlases[std::pair<atlas_type, texture_type>(type, data_type)] = texture;
            LOG(DEBUG) << "Texture added to atlas";
        }

        void texture_manager::add_texture_location(mc_texture_atlas_location &location) {
            std::string tex_name(location.name);
            glm::vec2 min_uv(location.min_u, location.min_v);
            glm::vec2 max_uv(location.max_u, location.max_v);

            texture_location tex_loc = {
                    min_uv,
                    max_uv
            };

            locations[tex_name] = tex_loc;
        }


        const texture_manager::texture_location &
        texture_manager::get_texture_location(const std::string &texture_name) {
            return locations[texture_name];
        }

        texture2D &texture_manager::get_texture_atlas(atlas_type atlas, texture_type type) {
            auto key = std::pair<atlas_type, texture_type>(atlas, type);
            return atlases[key];
        }

        int texture_manager::get_max_texture_size() {
            if(max_texture_size < 0) {
                glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

                // Hardcoded until I get everything moved to the proper thread
                max_texture_size = 8096;
            }
            return max_texture_size;
        }
    }
}
