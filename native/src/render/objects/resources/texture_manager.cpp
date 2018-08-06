/*!
 * \author David
 * \date 29-Apr-16.
 */

#include <algorithm>
#include <cmath>
#include <easylogging++.h>
#include "texture_manager.h"
#include "../../vulkan/render_context.h"
#include "../../../mc_interface/mc_objects.h"
#include <nova/profiler.h>

namespace nova {
    texture_manager::texture_manager(std::shared_ptr<render_context> context) : context(context) {
        LOG(DEBUG) << "Creating the Texture Manager";
        reset();
    }

    texture_manager::~texture_manager() {
        // gotta free up all the Vulkan textures
        reset();
    }

    void texture_manager::reset() {
        if(!atlases.empty()) {
            // Nothing to deallocate, let's just return
            return;
        }

        atlases.clear();
        locations.clear();
        auto usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
        atlases.emplace("NovaLightmap", texture2D("NovaLightmap", vk::Extent2D{16, 16}, vk::Format::eR8G8B8A8Unorm, usage,
                                            context, true));
        LOG(TRACE) << "Created lightmap";

        clear_dynamic_textures();
    }

    void texture_manager::add_texture(mc_atlas_texture &new_texture) {
        LOG(DEBUG) << "Adding texture " << new_texture.name << " (" << new_texture.width << "x" << new_texture.height << ")";
        std::string texture_name = new_texture.name;
        auto dimensions = vk::Extent2D{new_texture.width, new_texture.height};
        auto usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
        texture2D texture(texture_name, dimensions, vk::Format::eR8G8B8A8Unorm, usage, context);
        LOG(TRACE) << "Created texture object";

        std::vector<uint8_t> pixel_data((std::size_t) (new_texture.width * new_texture.height * new_texture.num_components));
        for(uint32_t i = 0; i < new_texture.width * new_texture.height * new_texture.num_components; i++) {
            pixel_data[i] = (uint8_t) new_texture.texture_data[i];
        }

        LOG(TRACE) << "Added pixel data to buffer";

        texture.set_data(pixel_data.data(), dimensions);

        LOG(TRACE) << "Sent texture data to GPU";

        LOG(DEBUG) << "Texture atlas " << texture_name << " is Vulkan texture " << texture.get_vk_image();
        atlases.emplace(texture_name, std::move(texture));
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
            return atlases.at(texture_name);
        }

        if(dynamic_tex_name_to_idx.find(texture_name) != dynamic_tex_name_to_idx.end()) {
            auto idx = dynamic_tex_name_to_idx.at(texture_name);
            if(dynamic_textures.size() > idx) {
                return dynamic_textures.at(idx);
            }
        }

        LOG(ERROR) << "Could not find texture " << texture_name;
        throw std::domain_error("Could not find texture " + texture_name);
    }

    int texture_manager::get_max_texture_size() {
        if(max_texture_size < 0) {
            max_texture_size = context->gpu.props.limits.maxImageDimension2D;

			LOG(DEBUG) << "max texturesize reported by gpu: " << max_texture_size;
        }
        return max_texture_size;
    }

    // Implementation based on RenderGraph::build_aliases from the Granite engine
    void texture_manager::create_dynamic_textures(const std::unordered_map<std::string, texture_resource> &textures,
                                                  const std::vector<render_pass> &passes, std::shared_ptr<swapchain_manager> swapchain) {
        NOVA_PROFILER_SCOPE;
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
                const input_textures& all_inputs = pass.texture_inputs.value();
                // color attachments
                for(const auto &input : all_inputs.color_attachments) {
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

                // shader-only textures
                for(const auto &input : all_inputs.bound_textures) {
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
                    auto& tex_range = resource_used_range[output.name];

                    if(pass_idx < tex_range.first_write_pass) {
                        tex_range.first_write_pass = pass_idx;

                    } else if(pass_idx > tex_range.last_write_pass) {
                        tex_range.last_write_pass = pass_idx;
                    }

                    if(std::find(resources_in_order.begin(), resources_in_order.end(), output.name) == resources_in_order.end()) {
                        resources_in_order.push_back(output.name);
                    }
                }
            }

            pass_idx++;
        }

        LOG(TRACE) << "Ordered resources";

        // Figure out which resources can be aliased
        std::unordered_map<std::string, std::string> aliases;

        for(size_t i = 0; i < resources_in_order.size(); i++) {
            const auto& to_alias_name = resources_in_order[i];
            LOG(TRACE) << "Determining if we can alias `" << to_alias_name << "`. Does it exist? " << (textures.find(to_alias_name) != textures.end());
            if(to_alias_name == "Backbuffer" || to_alias_name == "backbuffer") {
                // Yay special cases!
                continue;
            }

            const auto& to_alias_format = textures.at(to_alias_name).format;

            // Only try to alias with lower-indexed resources
            for(size_t j = 0; j < i; j++) {
                LOG(TRACE) << "Trying to alias it with resource at index " << j << " out of " << resources_in_order.size();
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

        LOG(TRACE) << "Figured out which resources can be aliased";

        auto swapchain_dimensions = swapchain->get_swapchain_extent();

        // For each texture:
        //  - If it isn't in the aliases map, create a new texture with its format and add it to the textures map
        //  - If it is in the aliases map, follow its chain of aliases

        for(const auto& named_texture : textures) {
            std::string texture_name = named_texture.first;
            while(aliases.find(texture_name) != aliases.end()) {
                LOG(TRACE) << "Resource " << texture_name << " is aliased with " << aliases[texture_name];
                texture_name = aliases[texture_name];
            }

            // We've found the first texture in this alias chain - let's create an actual texture for it if needed
            if(dynamic_tex_name_to_idx.find(texture_name) == dynamic_tex_name_to_idx.end()) {
                LOG(TRACE) << "Need to create it";
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

                auto usage = vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
                if(format.pixel_format == pixel_format_enum::DepthStencil || format.pixel_format == pixel_format_enum::Depth) {
                    usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;

                } else {
                    usage |= vk::ImageUsageFlagBits::eColorAttachment;
                }

                auto pixel_format = get_vk_format_from_pixel_format(format.pixel_format);
                LOG(DEBUG) << "Creating a texture with nova format " << pixel_format_enum::to_string(format.pixel_format) << " and Vulkan format " << vk::to_string(pixel_format) << ". It's being created for texture " << texture_name;
                auto tex = texture2D(std::to_string(dynamic_textures.size()), dimensions, pixel_format, usage, context);

                auto new_tex_index = dynamic_textures.size();
                dynamic_textures.emplace_back(std::move(tex));
                dynamic_tex_name_to_idx.emplace(texture_name, new_tex_index);

                LOG(TRACE) << "Added texture " << tex.get_name() << " to the dynamic textures";
                LOG(TRACE) << "set dynamic_texture_to_idx[" << texture_name << "] = " << new_tex_index;

            } else {
                LOG(TRACE) << "The physical resource already exists, so we're just gonna use that";
                // The texture we're aliasing already has a real texture behind it - so let's use that
                dynamic_tex_name_to_idx[named_texture.first] = dynamic_tex_name_to_idx[texture_name];
            }
        }
    }

    void texture_manager::clear_dynamic_textures() {
        LOG(TRACE) << "Cleared dynamic textures";
        dynamic_textures.resize(0);
        dynamic_tex_name_to_idx.erase(dynamic_tex_name_to_idx.begin(), dynamic_tex_name_to_idx.end());
    }

    bool texture_manager::is_texture_known(const std::string &texture_name) const {
        if(atlases.find(texture_name) != atlases.end()) {
            return true;
        }

        return dynamic_tex_name_to_idx.find(texture_name) != dynamic_tex_name_to_idx.end();
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
