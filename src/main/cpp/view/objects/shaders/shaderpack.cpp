/*!
 * \brief
 *
 * \author ddubois 
 * \date 20-Sep-16.
 */

#include <algorithm>
#include "shaderpack.h"

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

        shader_tree_node shaderpack::gbuffers_shaders = shader_tree_node(
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

        std::unordered_map<std::string, std::function<void(gometry_filter)> shaderpack::filter_modifying_functions = {
            { "block",              accept_block },
            { "not_block",          reject_block },
            { "selection_box",      accept_selection_box },
            { "not_selection_box",  reject_selection_box },
            { "entity",             accept_entity },
            { "not_entity",         reject_entity },
            { "particle",           accept_particle },
            { "not_particle",       reject_particle },
            { "sky_object",         accept_sky_object },
            { "not_sky_object",     reject_sky_object },
            { "solid",              accept_solid },
            { "not_solid",          reject_solid },
            { "transparent",        accept_transparent },
            { "not_transparent",    reject_transparent },
            { "cutout",             accept_cutout },
            { "not_cutout",         reject_cutout },
            { "emissive",           accept_emissive },
            { "not_emissive",       reject_emissive },
            { "damaged",            accept_damaged },
            { "not_damaged",        reject_damaged },
            { "everything_else",    accept_everything_else },
            { "nothing_else",       reject_everything_else }
        };

        nlohmann::json shaderpack::default_shaders_json;
        bool shaderpack::default_shaders_json_loaded = false;

        void shaderpack::load_default_shaders_json() {
           LOG(INFO) << "Loading default shader.json file";

           std::ifstream shaders_json("config/shaders.json");
           if(shaders_json.is_open()) {
               std::string buf;
               std::string accum;

               while(getline(shaders_json, buf)) {
                   accum += buf;   // Not the most effecient, but it works
               }

               default_shaders_json = nlohmann::json::parse(accum.c_str());
               default_shaders_json_loaded = true;

            } else {
                LOG(FATAL) << "Could not load default shaders.json file. Please make sure that the file 'shader.json' is present in the 'config' directory of your Minecraft home folder. If it is not, please download a new one.";
            }
        }

        void shaderpack::shaderpack(std::string shaderpack_name) {
            // First, check if there's a shaders.json file in the shaderpack.
            // If so, use it to tell us which shaders and filters we have. If
            // that file doesn't exist, load the default one
            
            if(!default_shaders_json_loaded) {
                load_default_shaders_json();
            }


        }

        void shaderpack::build_filters() {
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

        gl_shader_program& shaderpack::operator[](std::string key) {
            return loaded_shaders[key];
        }

        void shaderpack::set_shader_definitions(std::unordered_map<std::string, model::shader_definition>& definitions) {
            shaderpack_reading_guard.lock();
            shader_definitions = definitions;
            shaderpack_reading_guard.unlock();
        }

        std::unordered_map<std::string, gl_shader_program> &shaderpack::get_loaded_shaders() {
            return loaded_shaders;
        }

        void shaderpack::upload_shaders() {
            shaderpack_reading_guard.lock();
            for(const auto& shader : shader_definitions) {
                loaded_shaders.erase(shader.first);

                auto shader_program = gl_shader_program(shader.first, shader.second);

                loaded_shaders.emplace(shader.first, shader_program);
                loaded_shaders[shader.first].set_filter(filters[shader.first]);
            }
            shaderpack_reading_guard.unlock();
        }

    }
}
