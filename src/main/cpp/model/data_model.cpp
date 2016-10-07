/*!
 * \brief
 *
 * \author ddubois 
 * \date 04-Sep-16.
 */

#include "data_model.h"

#include "loaders/loaders.h"

namespace nova {
    namespace model {
        data_model::data_model() : render_settings("config/config.json") {
            render_settings.register_change_listener(this);

            render_settings.update_config_loaded();
            render_settings.update_config_changed();
        }

        gl_shader_program &data_model::get_shader_program(const std::string &program_name) {
            return loaded_shaderpack[program_name];
        }

        void data_model::on_config_change(nlohmann::json &config) {
            load_new_shaderpack(config["loadedShaderpack"]);
        }

        void data_model::on_config_loaded(nlohmann::json &config) {}

        void data_model::load_new_shaderpack(const std::string &new_shaderpack_name) noexcept {
            if(new_shaderpack_name != loaded_shaderpack_name) {
                try {
                    LOG(INFO) << "Loading shaderpack " << new_shaderpack_name;
                    loaded_shaderpack = load_shaderpack(new_shaderpack_name);
                    loaded_shaderpack_name = new_shaderpack_name;

                } catch(std::exception e) {
                    LOG(ERROR) << "Could not load shaderpack " << new_shaderpack_name << ". Reason: " << e.what();
                    loaded_shaderpack_name = "default";
                    loaded_shaderpack = load_shaderpack(loaded_shaderpack_name);
                }
            }
        }

        std::vector<gl_shader_program*> data_model::get_all_shaders() {
            auto all_shaders = std::vector<gl_shader_program*>{};
            for(auto& shader : loaded_shaderpack.get_loaded_shaders()) {
                all_shaders.push_back(&shader.second);
            }

            return all_shaders;
        }

        void data_model::trigger_config_update() {
            render_settings.update_config_loaded();
            render_settings.update_config_changed();
        }

        texture_manager &data_model::get_texture_manager() {
            return textures;
        }

        void data_model::set_gui_screen(mc_gui_screen *screen) {
            if(are_different_screens(*screen, cur_gui_screen)) {
                cur_gui_screen = *screen;
            }
        }

        settings& data_model::get_render_settings() {
            return render_settings;
        }

        bool are_different_screens(const mc_gui_screen &screen1, const mc_gui_screen &screen2) const {
            for(int i = 0; i < MAX_NUM_BUTTONS; i++) {
                if(are_different_buttons(screen1.buttons[i], screen2.buttons[i])) {
                    return true;
                }
            }

            return false;
        }

        bool are_different_buttons(const mc_gui_button &button1, const mc_gui_button &button2) const {
            bool same_rect = button1.x_position == button2.x_position &&
                             button1.y_position == button2.y_position &&
                             button1.width == button2.width &&
                             button1.height == button2.height;

            bool same_text = !are_different_strings(button1.text, button2.text);

            bool same_pressed = button1.is_pressed == button2.is_pressed;

            return !same_rect || !same_text || !same_pressed;
        }

        bool are_different_strings(const char *text1, const char *text2) const {
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

