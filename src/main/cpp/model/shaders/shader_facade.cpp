/*!
 * \brief
 *
 * \author ddubois 
 * \date 20-Sep-16.
 */

#include "shader_facade.h"
#include <algorithm>

namespace nova {
    namespace model {

        shader_tree_node::shader_tree_node(std::string name) : shader_name(name) {}

        shader_tree_node::shader_tree_node(std::string name, std::initializer_list<shader_tree_node> children) : shader_name(name), children(children) { }

        void shader_tree_node::calculate_filters(
                std::unordered_map<std::string, std::function<bool(render_object)>> orig_filters,    // Holy type signature, Batman!
                std::vector<std::string> loaded_shaders
        ) {
            filters.push_back(orig_filters[shader_name]);   // The filters for this shader always include the original filter for this shader

            for(auto& child : children) {
                child.calculate_filters(orig_filters, loaded_shaders);

                if(std::find(loaded_shaders.begin(), loaded_shaders.end(), child.shader_name) == loaded_shaders.end()) {
                    // Only add the child's filters to our own if the child's shader isn't loaded
                    filters.push_back(child.get_filter_function());
                }
            }
        }

        std::function<bool(render_object)> shader_tree_node::get_filter_function() {
            /*
             * When we calculate the filters for each shader_tree_node, we save a list of the filters that apply to a
             * given shader. This function coalesces those filters into a single function object and returns that
             */
            return [&](const auto& geom) {
                for(auto& filter : filters) {
                    if(filter(geom)) {
                        return true;
                    }
                }

                return false;
            };
        }

        void shader_tree_node::foreach_df(std::function<void(shader_tree_node&)> f) {
            f(*this);
            for(auto& child : children) {
                child.foreach_df(f);
            }
        }

        shader_tree_node shader_facade::gbuffers_shaders = shader_tree_node("gbuffers_basic", {
                shader_tree_node("gbuffers_skybasic"),
                shader_tree_node("gbuffers_textured", {
                        shader_tree_node("gbuffers_spidereyes"),
                        shader_tree_node("gbuffers_armor_glint"),
                        shader_tree_node("gbuffers_clouds"),
                        shader_tree_node("gbuffers_skytextured"),
                        shader_tree_node("gbuffers_textured_lit", {
                                shader_tree_node("gbuffers_entities"),
                                shader_tree_node("gbuffers_hand"),
                                shader_tree_node("gbuffers_weather"),
                                shader_tree_node("gbuffers_terrain", {
                                        shader_tree_node("gbuffers_damaged_block"),
                                        shader_tree_node("gbuffers_water"),
                                        shader_tree_node("gbuffers_block")
                                })
                        })
                })
        });

        void shader_facade::build_filters() {
            // First, define one filter per possible shader
            static std::unordered_map<std::string, std::function<bool(render_object)>> orig_filters = {
                    {"gui",                     [](const auto& geom) {return geom.type == geometry_type::gui;}},

                    {"shadow",                  [](const auto& geom) {return geom.type != geometry_type::fullscreen_quad;}},

                    {"gbuffers_basic",          [](const auto& geom) {return geom.type == geometry_type::selection_box;}},
                    {"gbuffers_textured",       [](const auto& geom) {return geom.type == geometry_type::particle;}},
                    {"gbuffers_textured_lit",   [](const auto& geom) {return geom.type == geometry_type::lit_particle || geom.name == "world_border";}},
                    {"gbuffers_skybasic",       [](const auto& geom) {return geom.name == "sky" || geom.name == "horizon" || geom.name == "stars" || geom.name == "void";}},
                    {"gbuffers_skytextured",    [](const auto& geom) {return geom.name == "sun" || geom.name == "moon";}},
                    {"gbuffers_clouds",         [](const auto& geom) {return geom.type == geometry_type::cloud;}},
                    {"gbuffers_terrain",        [](const auto& geom) {return geom.type == geometry_type::block && !geom.is_transparent;}},
                    {"gbuffers_damagedblock",   [](const auto& geom) {return geom.type == geometry_type::block && geom.damage_level > 0;}},
                    {"gbuffers_water",          [](const auto& geom) {return geom.type == geometry_type::block && geom.is_transparent;}},
                    {"gbuffers_block",          [](const auto& geom) {return geom.type == geometry_type::falling_block;}},
                    {"gbuffers_entities",       [](const auto& geom) {return geom.type == geometry_type::entity;}},
                    {"gbuffers_armor_glint",    [](const auto& geom) {return geom.type == geometry_type::glint;}},
                    {"gbuffers_spidereyes",     [](const auto& geom) {return geom.type == geometry_type::eyes;}},
                    {"gbuffres_hand",           [](const auto& geom) {return geom.type == geometry_type::hand;}},
                    {"gbuffers_weather",        [](const auto& geom) {return geom.type == geometry_type::weather;}},

                    {"composite",               [](const auto& geom) {return geom.type == geometry_type::fullscreen_quad;}},
                    {"composite1",              [](const auto& geom) {return geom.type == geometry_type::fullscreen_quad;}},
                    {"composite2",              [](const auto& geom) {return geom.type == geometry_type::fullscreen_quad;}},
                    {"composite3",              [](const auto& geom) {return geom.type == geometry_type::fullscreen_quad;}},
                    {"composite4",              [](const auto& geom) {return geom.type == geometry_type::fullscreen_quad;}},
                    {"composite5",              [](const auto& geom) {return geom.type == geometry_type::fullscreen_quad;}},
                    {"composite6",              [](const auto& geom) {return geom.type == geometry_type::fullscreen_quad;}},
                    {"composite7",              [](const auto& geom) {return geom.type == geometry_type::fullscreen_quad;}},
                    {"final",                   [](const auto& geom) {return geom.type == geometry_type::fullscreen_quad;}},
            };

            std::vector<std::string> loaded_shader_names;
            loaded_shader_names.reserve(loaded_shaders.size());
            for(auto kv : loaded_shaders) {
                loaded_shader_names.push_back(kv.first);
            }

            // Compute which filters apply to which shaders
            gbuffers_shaders.calculate_filters(orig_filters, loaded_shader_names);

            // Save the filter with the shader that uses it
            gbuffers_shaders.foreach_df(
                    [&](auto &node) {
                        if(loaded_shaders.find(node.shader_name) != loaded_shaders.end()) {
                            // If we've loaded this shader, let's set its filtering function as the filtering function for the shader
                            loaded_shaders[node.shader_name].set_filter(node.get_filter_function());
                        }
                    }
            );
        }

        gl_shader_program& shader_facade::operator[](std::string key) {
            return loaded_shaders[key];
        }

        void shader_facade::operator=(std::unordered_map<std::string, gl_shader_program>&& shaders) {
            loaded_shaders = shaders;
        }

        std::unordered_map<std::string, gl_shader_program> &shader_facade::get_loaded_shaders() {
            return loaded_shaders;
        }
    }
}
