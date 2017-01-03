/*!
 * \brief
 *
 * \author ddubois 
 * \date 04-Sep-16.
 */

#define ELPP_THREAD_SAFE
#include <easylogging++.h>

#include "data_model.h"

#include "loaders/loaders.h"

namespace nova {
    namespace model {
        data_model::data_model() : render_settings("config/config.json") {
            render_settings.register_change_listener(this);

            render_settings.update_config_loaded();
            render_settings.update_config_changed();
        }

        void data_model::on_config_change(nlohmann::json &config) {
            load_new_shaderpack(config["loadedShaderpack"]);
        }

        void data_model::on_config_loaded(nlohmann::json &config) {}

        void data_model::load_new_shaderpack(const std::string &new_shaderpack_name) noexcept {
            /*
             * Pretty sure loading a new shaderpack will necessitate a rebuild 
             * of all chunk geometry if the shaderpack has a shaders.json file.
             * We can fire off async tasks to rebuild the chunk geometry 
             * according to the new filters, but we'll have to halt rendering
             * until the first one returns. We can clear the chunk geometry 
             * cache when the new shaderpack is loaded in
             *
             * This means we'll probably want to store the filters somewhere
             * where they can be easily accessed. The data_loader is a decent
             * idea, but I'd rather have an actual geometry cache object. The
             * filters can be kept there
             *
             * Invalidating the chunk geometry cache when a new shaderpack is 
             * loaded won't be hard. Halting rendering until the shaderpack is 
             * loaded shouldn't be hard either - just don't return from this
             * method until the shaderpack has been completely loaded. The
             * question then becomes how to get the filters from the shaderpack
             * into the geometry builder. A method on shaderpack should work -
             * something that just returns a list of all geometry_filter 
             * objects that are active for a shader, and the shader that 
             * renders that filter. Geometry will be batched by shader, and
             * then when a shader renders it can simply ask for the geometry 
             * that corresponds to it.
             */
            if(new_shaderpack_name != loaded_shaderpack_name) {
                std::unordered_map<std::string, shader_definition> new_shaderpack;
                try {
                    LOG(INFO) << "Loading shaderpack " << new_shaderpack_name;
                    new_shaderpack = load_shaderpack(new_shaderpack_name);

                } catch(std::exception& e) {
                    LOG(ERROR) << "Could not load shaderpack " << new_shaderpack_name << ". Reason: " << e.what();
                    loaded_shaderpack_name = "default";
                    new_shaderpack = load_shaderpack(loaded_shaderpack_name);
                }

                loaded_shaderpack_name = new_shaderpack_name;
                loaded_shaderpack.set_resource(new_shaderpack);
            }
        }

        loaded_resource<std::unordered_map<std::string, shader_definition>>& data_model::get_loaded_shaderpack() {
            return loaded_shaderpack;
        }

        void data_model::trigger_config_update() {
            render_settings.update_config_loaded();
            render_settings.update_config_changed();
        }

        texture_manager &data_model::get_texture_manager() {
            return textures;
        }

        void data_model::set_gui_screen(mc_gui_screen *screen) {
            LOG(DEBUG) << "Called set_gui_screen";
            bool different = are_different_screens(*screen, cur_gui_screen);
            if(different) {
                LOG(DEBUG) << "Rebuilding GUI geometry";
                cur_gui_screen = *screen;
                meshes.build_geometry(cur_gui_screen);
                LOG(DEBUG) << "Geometry re-built";
            }
        }

        settings& data_model::get_render_settings() {
            return render_settings;
        }

        mesh_builder &data_model::get_mesh_builder() {
            return meshes;
        }

        bool are_different_screens(const mc_gui_screen &screen1, const mc_gui_screen &screen2) {
            if(screen1.num_buttons != screen2.num_buttons) {
                return true;
            }

            for(int i = 0; i < MAX_NUM_BUTTONS; i++) {
                if(i >= screen1.num_buttons || i >= screen2.num_buttons) {
                    break;
                }
                LOG(TRACE) << "Checking button " << i << " for similarity";
                if(are_different_buttons(screen1.buttons[i], screen2.buttons[i])) {
                    LOG(TRACE) << "Button " << i << " is different";
                    return true;
                }

                LOG(TRACE) << "Button " << i << " is the same in both screens";
            }

            return false;
        }

        bool are_different_buttons(const mc_gui_button &button1, const mc_gui_button &button2) {
            bool same_rect = button1.x_position == button2.x_position &&
                             button1.y_position == button2.y_position &&
                             button1.width == button2.width &&
                             button1.height == button2.height;

            bool same_text = !are_different_strings(button1.text, button2.text);

            bool same_pressed = button1.is_pressed == button2.is_pressed;

            return !same_rect || !same_text || !same_pressed;
        }

        bool are_different_strings(const char *text1, const char *text2) {
            if(text1 == nullptr && text2 == nullptr) {
                // They're both null, and null equals null, so they're the same
                // If this causes problems I'll change it
                return false;
            }

            if(text1 == nullptr) {
                return true;
            }

            if(text2 == nullptr) {
                return true;
            }

            return strcmp(text1, text2) != 0;
        }
    }
}

