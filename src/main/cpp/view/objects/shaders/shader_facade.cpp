/*!
 * \brief
 *
 * \author ddubois 
 * \date 20-Sep-16.
 */

#include <algorithm>
#include "shader_facade.h"

namespace nova {
    namespace view {

        shader_tree_node::shader_tree_node(std::string name, geometry_filter filter) :
                shader_name(name), filter(filter) {}

        shader_tree_node::shader_tree_node(std::string name, geometry_filter filter, std::vector<shader_tree_node> children) :
                shader_name(name), filter(filter), children(children) { }

        void shader_tree_node::calculate_filters(std::vector<std::string> loaded_shaders) {
            filters.push_back(filter);   // The filters for this shader always include the original filter for this shader

            for(auto& child : children) {
                child.calculate_filters(loaded_shaders);

                if(std::find(loaded_shaders.begin(), loaded_shaders.end(), child.shader_name) == loaded_shaders.end()) {
                    // Only add the child's filters to our own if the child's shader isn't loaded
                    filters.push_back(child.get_filter());
                }
            }

            filter = [&](const render_object& geom) {
                for(auto& filter : filters) {
                    if(filter(geom)) {
                        return true;
                    }
                }

                return false;
            };
        }

        geometry_filter& shader_tree_node::get_filter() {
            return filter;
        }

        void shader_tree_node::foreach_df(std::function<void(shader_tree_node&)> f) {
            f(*this);
            for(auto& child : children) {
                child.foreach_df(f);
            }
        }

        shader_tree_node shader_facade::gbuffers_shaders = shader_tree_node(
                "gbuffers_basic",
                geometry_filter().selection_box().build(),
                {
                shader_tree_node(
                        "gbuffers_skybasic",
                        geometry_filter().sky_object().build()
                ),
                shader_tree_node(
                        "gbuffers_textured",
                        geometry_filter().particle().build(),
                        {
                        shader_tree_node(
                                "gbuffers_spidereyes",
                                geometry_filter().add_geometry_type(geometry_type::eyes).build()
                        ),
                        shader_tree_node(
                                "gbuffers_armor_glint",
                                geometry_filter().add_geometry_type(geometry_type::glint).build()
                        ),
                        shader_tree_node(
                                "gbuffers_clouds",
                                geometry_filter().add_geometry_type(geometry_type::cloud).build()
                        ),
                        shader_tree_node(
                                "gbuffers_skytextured",
                                geometry_filter().add_name("sun").add_name("moon").build()
                        ),
                        shader_tree_node(
                                "gbuffers_textured_lit",
                                geometry_filter().add_geometry_type(geometry_type::lit_particle).add_name("world_border").build(),
                                {
                                shader_tree_node(
                                        "gbuffers_entities",
                                        geometry_filter().entity().build()
                                    ),
                                shader_tree_node(
                                        "gbuffers_hand",
                                        geometry_filter().add_geometry_type(geometry_type::hand).build()
                                ),
                                shader_tree_node(
                                        "gbuffers_weather",
                                        geometry_filter().add_geometry_type(geometry_type::weather).build()
                                ),
                                shader_tree_node(
                                        "gbuffers_terrain",
                                        geometry_filter().add_geometry_type(geometry_type::block).not_transparent().build(),
                                        {
                                        shader_tree_node(
                                                "gbuffers_damagedblock",
                                                geometry_filter().block().damaged().build()
                                        ),
                                        shader_tree_node(
                                                "gbuffers_water",
                                                geometry_filter().block().transparent().build()
                                        ),
                                        shader_tree_node(
                                                "gbuffers_block",
                                                geometry_filter().block().entity().build()
                                        )
                                })
                        })
                })
        });

        void shader_facade::build_filters() {
            // Which shaders are actually loaded?
            std::vector<std::string> loaded_shader_names;

            shaderpack_reading_guard.lock();
            loaded_shader_names.reserve(shader_definitions.size());
            for(auto& kv : shader_definitions) {
                loaded_shader_names.push_back(kv.first);
            }

            // Compute which filters apply to which shaders
            gbuffers_shaders.calculate_filters(loaded_shader_names);

            // Save the filter with the shader that uses it
            gbuffers_shaders.foreach_df(
                    [&](auto &node) {
                        if(shader_definitions.find(node.shader_name) != shader_definitions.end()) {
                            // If we've loaded this shader, let's set its filtering function as the filtering function for the shader
                            filters[node.shader_name] = node.get_filter_function();
                        }
                    }
            );

            // Save the filters for the shaders that don't exist in the gbuffers tree
            for(auto& item : shader_definitions) {
                if(item.first == "gui") {
                    filters["gui"] = [](const render_object& geom) {return geom.type == geometry_type::gui;};

                } else if(item.first.find("composite") == 0 || item.first == "final") {
                    filters[item.first] = [](const render_object& geom) {return geom.type == geometry_type::fullscreen_quad;};

                } else if(item.first.find("shadow") == 0) {
                    filters[item.first] = [](const render_object& geom) {return geom.type != geometry_type::fullscreen_quad;};
                }
            }
            shaderpack_reading_guard.unlock();
        }

        gl_shader_program& shader_facade::operator[](std::string key) {
            return loaded_shaders[key];
        }

        void shader_facade::set_shader_definitions(std::unordered_map<std::string, model::shader_definition>& definitions) {
            shaderpack_reading_guard.lock();
            shader_definitions = definitions;
            shaderpack_reading_guard.unlock();
        }

        std::unordered_map<std::string, gl_shader_program> &shader_facade::get_loaded_shaders() {
            return loaded_shaders;
        }

        void shader_facade::upload_shaders() {
            shaderpack_reading_guard.lock();
            for(const auto& shader : shader_definitions) {
                loaded_shaders.erase(shader.first);

                auto shader_program = gl_shader_program(shader.first, shader.second);

                loaded_shaders.emplace(shader.first, shader_program);
                loaded_shaders[shader.first].set_filter(filters[shader.first]);
            }
            shaderpack_reading_guard.unlock();
        }

        geometry_filter& geometry_filter::block() {
            should_be_block = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_block() {
            should_be_block = false;
            return *this;
        }

        geometry_filter &geometry_filter::entity() {
            should_be_entity = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_entity() {
            should_be_entity = false;
            return *this;
        }

        geometry_filter &geometry_filter::particle() {
            should_be_particle = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_particle() {
            should_be_particle = false;
            return *this;
        }

        geometry_filter &geometry_filter::sky_object() {
            should_be_sky_object = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_sky_object() {
            should_be_sky_object = false;
            return *this;
        }

        geometry_filter &geometry_filter::add_geometry_type(geometry_type type) {
            geometry_types.push_back(type);
            return *this;
        }

        geometry_filter &geometry_filter::add_name(std::string name) {
            names.push_back(name);
            return *this;
        }

        geometry_filter &geometry_filter::add_name_part(std::string name_part) {
            name_parts.puch_back(name_part);
            return *this;
        }

        geometry_filter &geometry_filter::transparent() {
            should_be_transparent = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_transparent() {
            should_be_transparent = false;
            return *this;
        }

        geometry_filter &geometry_filter::cutout() {
            should_be_cutout = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_cutout() {
            should_be_cutout = false;
            return *this;
        }

        geometry_filter &geometry_filter::emissive() {
            should_be_emissive = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_emissive() {
            should_be_emissive = false;
            return *this;
        }

        geometry_filter geometry_filter::build() {
            return *this;
        }

        geometry_filter &geometry_filter::damaged() {
            should_be_damaged = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_damaged() {
            should_be_damaged = false;
            return *this;
        }

        geometry_filter &geometry_filter::selection_box() {
            should_be_selection_box = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_selection_box() {
            should_be_selection_box = false;
            return *this;
        }
    }
}
