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
#include "texture_manager.h"

namespace nova {
    namespace model {
        /*!
         * \brief Holds all the data in the Model, providing methods to access the various pieces of data
         *
         * This is also the class that the Minecraft interface goes through to send data to the Nova Renderer
         */
        class data_model : public iconfig_listener {
        public:
            data_model();

            gl_shader_program& get_shader_program(const std::string& program_name);

            /*!
             * \brief Explicitly tells the render_settings to send its data to all its registered listeners
             *
             * Useful when you register a new thing and want to make sure it gets all the latest data
             */
            void trigger_config_update();

            void set_gui_screen(mc_gui_screen* screen);

            std::vector<gl_shader_program*> get_all_shaders();

            texture_manager& get_texture_manager();

            settings& get_render_settings();

            /* From iconfig_listener */

            void on_config_loaded(nlohmann::json& config);

            void on_config_change(nlohmann::json& config);

        private:
            settings render_settings;
            shader_facade loaded_shaderpack;
            texture_manager textures;
            std::string loaded_shaderpack_name;
            mc_gui_screen cur_gui_screen;

            void load_new_shaderpack(const std::string &new_shaderpack_name) noexcept;
        };

        /*!
         * \brief Compares two mc_gui_screen objects, determining if they represent the same visual data
         *
         * I'l like to have had this function in the same header file as the mc_gui_screen struct. However, I need
         * mc_gui_screen to be a C struct so I can properly assign to it from Java. The compiler yelled at me about "You
         * can't compare structs" so I couldn't use the == operator and here we are.
         */
        bool are_different_screens(const mc_gui_screen &screen1, const mc_gui_screen &screen2) const;

        /*!
         * \brief Determines whether or not the two given buttons are the same
         *
         * Two buttons are the same if they have the same position, size, and pressed status. If they can be drawn using
         * the exact same geometry and texture, then they are the same.
         *
         * \param button1 The first button to compare
         * \param button2 The second button to compare
         *
         * \return True if the buttons are the same, false otherwise
         */
        bool are_different_buttons(const mc_gui_button &button1, const mc_gui_button &button2) const;

        /*!
         * \brief Compares two C strings, taking special care to not crash if one or both of the strings is null
         *
         * \param text1 The first string to compare
         * \param text2  The second string to compare
         * \return True is the strings are different, false otherwise
         */
        bool are_different_strings(const char *text1, const char *text2) const;
    }
}

#endif //RENDERER_MODEL_H
