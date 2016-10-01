/*!
 * \author David
 * \date 05-Jul-16.
 */

#ifndef RENDERER_UBO_MANAGER_H
#define RENDERER_UBO_MANAGER_H

#include <string>
#include <unordered_map>
#include <json.hpp>
#include <model/shaders/gl_shader_program.h>

#include "model/settings.h"
#include "model/gl/gl_uniform_buffer.h"
#include "model/shaders/uniform_buffer_definitions.h"

namespace nova {
    namespace view {
        /*!
         * \brief Holds all the uniform buffers that Nova needs to use
         *
         * The Uniform Buffer Store is kinda nice because it registers itself as a configuration change listenee,
         * meaning that it will receive updates whenever a new configuratin is available. These updates are (currently)
         * uploaded by the "update" stage of the renderer
         *
         * Ideally, all transfers of data from the CPU to GPU will happen in a separate thread, and the render thread
         * will do nothing except dispatch rendering commands
         */
        class uniform_buffer_store : public nova::model::iconfig_listener {
        public:
            /*!
             * \brief Creates all the uniform buffers that we need, and loads the binding points from the config/data file
             */
            uniform_buffer_store();

            model::gl_uniform_buffer &operator[](std::string name);

            void register_all_buffers_with_shader(model::gl_shader_program &shader) const noexcept;

            void update();

            /*
             * Inherited from iconfig_listener
             */
            virtual void on_config_change(nlohmann::json &new_config);

            virtual void on_config_loaded(nlohmann::json &config);

        private:
            // Saved so that we can reference these values later
            nlohmann::json config;

            std::unordered_map<std::string, model::gl_uniform_buffer> buffers;

            model::gui_uniforms gui_uniforms;
            model::per_frame_uniforms per_frame_uniforms;

            void set_bind_points(nlohmann::json &config);

            void create_ubos();

            void update_gui_uniforms();

            void update_per_frame_uniforms();
        };
    }
}

#endif //RENDERER_UBO_MANAGER_H
