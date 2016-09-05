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
        }

        gl_shader_program &data_model::get_shader_program(const std::string &program_name) {
            return loaded_shaderpack[program_name];
        }

        void data_model::on_config_change(nlohmann::json &config) {
            load_new_shaderpack(config["loadedShaderpack"]);
        }

        void data_model::on_config_loaded(nlohmann::json &config) {}

        void data_model::load_new_shaderpack(const std::string &new_shaderpack_name) {
            if(new_shaderpack_name != loaded_shaderpack_name) {
                try {
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
            for(auto& shader : loaded_shaderpack) {
                all_shaders.push_back(&shader.second);
            }

            return all_shaders;
        }
    }
}

