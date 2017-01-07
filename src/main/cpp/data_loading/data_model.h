/*!
 * \brief
 *
 * \author ddubois 
 * \date 04-Sep-16.
 */

#ifndef RENDERER_MODEL_H
#define RENDERER_MODEL_H

#include <atomic>
#include "data_loading/geometry_cache/builders/mesh_builder.h"
#include "data_loading/loaders/shader_source_structs.h"
#include "view/objects/shaders/shaderpack.h"
#include "settings.h"
#include "texture_manager.h"


namespace nova {
    /*!
     * \brief Holds all the data in the Model, providing methods to access the various pieces of data
     *
     * This is also the class that the Minecraft interface goes through to send data to the Nova Renderer
     */
    class data_model : public iconfig_listener {
    public:
        data_model();

        /*!
         * \brief Explicitly tells the render_settings to send its data to all its registered listeners
         *
         * Useful when you register a new thing and want to make sure it gets all the latest data
         */
        void trigger_config_update();

        /*!
         * \brief Sets the current GUI screen
         *
         * Note that this method can trigger a GUI geometry rebuild if the sent screen is different from the
         * current screen. This may or may not be important to know, but I feel like I should say it
         *
         * \param screen
         */
        void set_gui_screen(mc_gui_screen *screen);

        shaderpack &get_loaded_shaderpack();

        texture_manager &get_texture_manager();

        settings &get_render_settings();

        mesh_builder &get_mesh_builder();

        /* From iconfig_listener */

        void on_config_loaded(nlohmann::json &config);

        void on_config_change(nlohmann::json &config);

    private:
        settings render_settings;
        shaderpack loaded_shaderpack;
        texture_manager textures;
        std::string loaded_shaderpack_name;
        mesh_builder meshes;

        void load_new_shaderpack(const std::string &new_shaderpack_name) noexcept;
    };
}

#endif //RENDERER_MODEL_H
