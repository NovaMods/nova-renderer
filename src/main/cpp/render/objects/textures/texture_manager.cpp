/*!
 * \author David
 * \date 29-Apr-16.
 */

#include <algorithm>
#include <easylogging++.h>
#include "texture_manager.h"

namespace nova {
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
        if(!atlases.empty()) {
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

    void texture_manager::add_texture(mc_atlas_texture &new_texture, texture_type data_type) {
        LOG(DEBUG) << "Creating a Texture2D for the " << data_type.to_string() << " atlas";
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

        atlases[data_type] = texture;
        LOG(DEBUG) << "Texture added to atlas";
        LOG(DEBUG) << "Texture atlas " << data_type.to_string() << " is OpenGL texture " << texture.get_gl_name();
    }

    void texture_manager::add_texture_location(mc_texture_atlas_location &location) {
        texture_location tex_loc = {
                { location.min_u, location.min_v },
                { location.max_u, location.max_v }
        };

        locations[location.name] = tex_loc;
    }


    const texture_manager::texture_location &texture_manager::get_texture_location(const std::string &texture_name) {
        return locations[texture_name];
    }

    texture2D &texture_manager::get_texture_atlas(texture_type type) {
        return atlases[type];
    }

    int texture_manager::get_max_texture_size() {
        if(max_texture_size < 0) {
            glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

			LOG(DEBUG) << "max texturesize reported by gpu: " << max_texture_size;
        }
        return max_texture_size;
    }
}
