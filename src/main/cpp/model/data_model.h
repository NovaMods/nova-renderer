/*!
 * \brief
 *
 * \author ddubois 
 * \date 04-Sep-16.
 */

#ifndef RENDERER_MODEL_H
#define RENDERER_MODEL_H

#include "model/shaders/shader_facade.h"
#include "settings.h"
#include "model/shaders/gl_shader_program.h"

namespace nova {
    namespace model {
        /*!
         * \brief Holds all the data in the Model, providing methods to access the various pieces of data
         */
        class data_model : public iconfig_listener {
        public:
            settings render_settings;
            shader_facade loaded_shaderpack;

            data_model();

            gl_shader_program& get_shader_program(const std::string& program_name);

            /*!
             * \brief Explicitly tells the render_settings to send its data to all its registered listeners
             *
             * Useful when you register a new thing and want to make sure it gets all the latest data
             */
            void trigger_config_update();

            /* From iconfig_listener */

            void on_config_loaded(nlohmann::json& config);

            void on_config_change(nlohmann::json& config);

            std::vector<gl_shader_program*> get_all_shaders();

        private:
            std::string loaded_shaderpack_name;

            void load_new_shaderpack(const std::string &new_shaderpack_name) noexcept;
        };
    }
}

#endif //RENDERER_MODEL_H
