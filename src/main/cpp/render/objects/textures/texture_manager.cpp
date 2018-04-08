/*!
 * \author David
 * \date 29-Apr-16.
 */

#include <algorithm>
#include <easylogging++.h>
#include "texture_manager.h"
#include "../../vulkan/render_context.h"
#include "../../../mc_interface/mc_objects.h"
#include "../../nova_renderer.h"

namespace nova {
    texture_manager::texture_manager(std::shared_ptr<render_context> context) : context(context) {
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

        atlases["lightmap"] = texture2D(vk::Extent2D{16, 16}, vk::Format::eR8G8B8A8Unorm, context);

        clear_dynamic_textures();
    }

    void texture_manager::update_texture(std::string texture_name, void* data, glm::ivec2 &size) {
        auto &texture = atlases[texture_name];
        texture.set_data(data, vk::Extent2D{static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)});
    }

    void texture_manager::add_texture(mc_atlas_texture &new_texture) {
        LOG(DEBUG) << "Adding texture " << new_texture.name << " (" << new_texture.width << "x" << new_texture.height << ")";
        std::string texture_name = new_texture.name;
        LOG(TRACE) << "Saved texture name";
        auto dimensions = vk::Extent2D{new_texture.width, new_texture.height};
        texture2D texture{dimensions, vk::Format::eR8G8B8A8Unorm, context};
        LOG(TRACE) << "Created texture object";
        texture.set_name(texture_name);

        std::vector<float> pixel_data((std::size_t) (new_texture.width * new_texture.height * new_texture.num_components));
        for(auto i = 0; i < new_texture.width * new_texture.height * new_texture.num_components; i++) {
            pixel_data[i] = float(new_texture.texture_data[i]) / 255.0f;
        }

        LOG(TRACE) << "Added pixel data to buffer";

        texture.set_data(pixel_data.data(), dimensions);

        LOG(TRACE) << "Sent texture data to GPU";

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
        if(atlases.find(texture_name) != atlases.end()) {
            return atlases[texture_name];
        }

        if(dynamic_tex_name_to_idx.find(texture_name) != dynamic_tex_name_to_idx.end()) {
            auto idx = dynamic_tex_name_to_idx[texture_name];
            if(dynamic_textures.size() > idx) {
                return dynamic_textures[idx];
            }
        }

        LOG(ERROR) << "Could not find texture " << texture_name;
        throw std::domain_error("Could not find texture " + texture_name);
    }

    int texture_manager::get_max_texture_size() {
        if(max_texture_size < 0) {
            max_texture_size = nova_renderer::instance->get_render_context()->gpu.props.limits.maxImageDimension2D;

			LOG(DEBUG) << "max texturesize reported by gpu: " << max_texture_size;
        }
        return max_texture_size;
    }

    // Implementation based on RenderGraph::build_aliases from the Granite engine
    void texture_manager::create_dynamic_textures(const std::unordered_map<std::string, texture_resource> &textures,
                                                  const std::vector<render_pass> &passes) {
        // For each texture in the passes, try to assign it to an existing resource
        // We'll basically create a list of which texture resources can be assigned to each physical resource
        // We want to alias textures. We can alias texture A and B if all reads from A finish before all writes to B AND
        // if A and B have the same format and dimension
        // Maybe we should make a list of things with the same format and dimension?

        clear_dynamic_textures();

        struct range {
            uint32_t first_write_pass = ~0u;
            uint32_t last_write_pass = 0;
            uint32_t first_read_pass = ~0u;
            uint32_t last_read_pass = 0;

            bool has_writer() const
            {
                return first_write_pass <= last_write_pass;
            }

            bool has_reader() const
            {
                return first_read_pass <= last_read_pass;
            }

            bool is_used() const
            {
                return has_writer() || has_reader();
            }

            bool can_alias() const
            {
                // If we read before we have completely written to a resource we need to preserve it, so no alias is possible.
                return !(has_reader() && has_writer() && first_read_pass <= first_write_pass);
            }

            unsigned last_used_pass() const
            {
                unsigned last_pass = 0;
                if (has_writer())
                    last_pass = std::max(last_pass, last_write_pass);
                if (has_reader())
                    last_pass = std::max(last_pass, last_read_pass);
                return last_pass;
            }

            unsigned first_used_pass() const
            {
                unsigned first_pass = ~0u;
                if (has_writer())
                    first_pass = std::min(first_pass, first_write_pass);
                if (has_reader())
                    first_pass = std::min(first_pass, first_read_pass);
                return first_pass;
            }

            bool is_disjoint_with(const range& other) const {
                if (!is_used() || !other.is_used())
                    return false;
                if (!can_alias() || !other.can_alias())
                    return false;

                bool left = last_used_pass() < other.first_used_pass();
                bool right = other.last_used_pass() < first_used_pass();
                return left || right;
            }
        };

        // Look at what range of render passes each resource is used in
        std::unordered_map<std::string, range> resource_used_range;
        std::vector<std::string> resources_in_order;

        uint32_t pass_idx = 0;
        for(const auto& pass : passes) {
            if(pass.texture_inputs) {
                for(const auto &input : pass.texture_inputs.value()) {
                    auto& tex_range = resource_used_range[input];

                    if(pass_idx < tex_range.first_write_pass) {
                        tex_range.first_write_pass = pass_idx;

                    } else if(pass_idx > tex_range.last_write_pass) {
                        tex_range.last_write_pass = pass_idx;
                    }

                    if(std::find(resources_in_order.begin(), resources_in_order.end(), input) == resources_in_order.end()) {
                        resources_in_order.push_back(input);
                    }
                }
            }

            if(pass.texture_outputs) {
                for(const auto &output : pass.texture_outputs.value()) {
                    auto& tex_range = resource_used_range[output];

                    if(pass_idx < tex_range.first_write_pass) {
                        tex_range.first_write_pass = pass_idx;

                    } else if(pass_idx > tex_range.last_write_pass) {
                        tex_range.last_write_pass = pass_idx;
                    }

                    if(std::find(resources_in_order.begin(), resources_in_order.end(), output) == resources_in_order.end()) {
                        resources_in_order.push_back(output);
                    }
                }
            }

            pass_idx++;
        }

        // Figure out which resources can be aliased
        std::unordered_map<std::string, std::string> aliases;

        for(auto i = 0; i < resources_in_order.size(); i++) {
            const auto& to_alias_name = resources_in_order[i];
            if(to_alias_name == "Backbuffer" || to_alias_name == "backbuffer") {
                // Yay special cases!
                continue;
            }
            const auto& to_alias_format = textures.at(to_alias_name).format;

            // Only try to alias with lower-indexed resources
            for(auto j = 0; j < i; j++) {
                const auto& try_alias_name = resources_in_order[j];
                if(resource_used_range[to_alias_name].is_disjoint_with(resource_used_range[try_alias_name])) {
                    // They can be aliased if they have the same format
                    const auto& try_alias_format = textures.at(try_alias_name).format;
                    if(to_alias_format == try_alias_format) {
                        aliases[to_alias_name] = try_alias_name;
                    }
                }
            }
        }

        auto swapchain_dimensions = context->swapchain_extent;

        // For each texture:
        //  - If it isn't in the aliases map, create a new texture with its format and add it to the textures map
        //  - If it is in the aliases map, follow its chain of aliases

        for(const auto& named_texture : textures) {
            std::string texture_name = named_texture.first;
            while(aliases.find(texture_name) != aliases.end()) {
                texture_name = aliases[texture_name];
            }

            // We've found the first texture in this alias chain - let's create an actual texture for it if needed
            if(dynamic_tex_name_to_idx.find(texture_name) != dynamic_tex_name_to_idx.end()) {
                // The texture we're all aliasing doesn't have a real texture yet. Let's fix that
                const texture_format& format = textures.at(texture_name).format;

                vk::Extent2D dimensions;
                if(format.dimension_type == texture_dimension_type_enum::Absolute) {
                    dimensions = vk::Extent2D{static_cast<uint32_t>(format.width), static_cast<uint32_t>(format.height)};

                } else {
                    dimensions = swapchain_dimensions;
                    dimensions.width *= format.width;
                    dimensions.height *= format.height;
                }

                auto pixel_format = get_vk_format_from_pixel_format(format.pixel_format);
                auto tex = texture2D{dimensions, pixel_format, context};

                auto new_tex_index = dynamic_textures.size();
                dynamic_textures.push_back(tex);
                dynamic_tex_name_to_idx[texture_name] = new_tex_index;
                dynamic_tex_name_to_idx[named_texture.first] = new_tex_index;

            } else {
                // The texture we're aliasing already has a real texture behind it - so let's use that
                dynamic_tex_name_to_idx[named_texture.first] = dynamic_tex_name_to_idx[texture_name];
            }
        }
    }

    void texture_manager::clear_dynamic_textures() {
        dynamic_textures.resize(0);
        dynamic_tex_name_to_idx.erase(dynamic_tex_name_to_idx.begin(), dynamic_tex_name_to_idx.end());
    }

    vk::Format get_vk_format_from_pixel_format(pixel_format_enum format) {
        switch(format) {
            case pixel_format_enum::RGB8:
                return vk::Format::eR8G8B8Unorm;
            case pixel_format_enum::RGBA8:
                return vk::Format::eR8G8B8A8Unorm;
            case pixel_format_enum::RGB16F:
                return vk::Format::eR16G16B16Sfloat;
            case pixel_format_enum::RGBA16F:
                return vk::Format::eR16G16B16A16Sfloat;
            case pixel_format_enum::RGB32F:
                return vk::Format::eR32G32B32Sfloat;
            case pixel_format_enum::RGBA32F:
                return vk::Format::eR32G32B32A32Sfloat;
            case pixel_format_enum::Depth:
                return vk::Format::eD32Sfloat;
            case pixel_format_enum::DepthStencil:
                return vk::Format::eD24UnormS8Uint;
            default:
                LOG(WARNING) << "Could not determine Vulkan format for pixel format " << pixel_format_enum::to_string(format);
                return vk::Format::eR8G8B8Unorm;
        }
    }
}
