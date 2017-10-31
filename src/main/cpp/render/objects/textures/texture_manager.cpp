/*!
 * \author David
 * \date 29-Apr-16.
 */

#include <algorithm>
#include <easylogging++.h>
#include "texture_manager.h"
#include "../../vulkan/render_context.h"
#include "../../../mc_interface/mc_objects.h"

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

        for(auto& tex : atlases) {
            tex.second.destroy();
        }


        atlases.clear();
        locations.clear();

        atlases["lightmap"] = texture2D();
    }

    void texture_manager::update_texture(std::string texture_name, void* data, glm::ivec2 &size, GLenum format, GLenum type, GLenum internal_format) {
        auto &texture = atlases[texture_name];
        //texture.set_data(data, size, format, type, internal_format);
    }

    void texture_manager::add_texture(mc_atlas_texture &new_texture) {
        LOG(INFO) << "Adding texture " << new_texture.name << " (" << new_texture.width << "x" << new_texture.height << ")";
        std::string texture_name = new_texture.name;
        texture2D texture;
        texture.set_name(texture_name);

        std::vector<float> pixel_data(
                (std::size_t) (new_texture.width * new_texture.height * new_texture.num_components));
        for(int i = 0; i < new_texture.width * new_texture.height * new_texture.num_components; i++) {
            pixel_data[i] = float(new_texture.texture_data[i]) / 255.0f;
        }

        auto dimensions = vk::Extent2D{new_texture.width, new_texture.height};

        texture.set_data(pixel_data.data(), dimensions, vk::Format::eR8G8B8A8Unorm);

        atlases[texture_name] = texture;
        LOG(DEBUG) << "Texture atlas " << texture_name << " is Vulkan texture " << texture.get_vk_image();
    }

    void texture_manager::add_texture_location(mc_texture_atlas_location &location) {
        texture_location tex_loc = {
                { location.min_u, location.min_v },
                { location.max_u, location.max_v }
        };

        locations[location.name] = tex_loc;
    }


    const texture_manager::texture_location texture_manager::get_texture_location(const std::string &texture_name) {
        // If we haven't explicitly added a texture location for this texture, let's just assume that the texture isn't
        // in an atlas and thus covers the whole (0 - 1) UV space

        if(locations.find(texture_name) != locations.end()) {
            return locations[texture_name];

        } else {
            return {{0, 0}, {1, 1}};
        }
    }

    texture2D &texture_manager::get_texture(std::string texture_name) {
        return atlases[texture_name];
    }

    int texture_manager::get_max_texture_size() {
        if(max_texture_size < 0) {
            max_texture_size = render_context::instance.gpu.props.limits.maxImageDimension2D;

			LOG(DEBUG) << "max texturesize reported by gpu: " << max_texture_size;
        }
        return max_texture_size;
    }
}
