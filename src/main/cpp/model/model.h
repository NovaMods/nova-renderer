/*!
 * \brief
 *
 * \author ddubois 
 * \date 04-Sep-16.
 */

#ifndef RENDERER_MODEL_H
#define RENDERER_MODEL_H

#include "settings.h"
#include "gl/gl_shader_program.h"

namespace nova {
    namespace model {
        /*!
         * \brief Holds all the data in the Model, providing methods to access the various pieces of data
         */
        class data_model : public iconfig_listener {
        public:
            settings render_settings;

            data_model();

            gl_shader_program& get_shader_program(const std::string& program_name);

            /* From iconfig_listener */

            void on_config_loaded(nlohmann::json& config);

            void on_config_changed(nlohmann::json& config);

            std::vector<gl_shader_program&> get_all_shaders();

        private:
            std::unordered_map<std::string, gl_shader_program> loaded_shaderpack;
            std::string loaded_shaderpack_name;

            void load_new_shaderpack(const std::string &new_shaderpack_name) noexcept;
        };
    }
}

#endif //RENDERER_MODEL_H
