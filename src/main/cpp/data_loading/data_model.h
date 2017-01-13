/*!
 * \brief
 *
 * \author ddubois 
 * \date 04-Sep-16.
 */

#ifndef RENDERER_MODEL_H
#define RENDERER_MODEL_H

#include <atomic>
#include "settings.h"
#include "../render/objects/shaders/shaderpack.h"
#include "../render/objects/textures/texture_manager.h"
#include "geometry_cache/builders/mesh_builder.h"


namespace nova {
    /*!
     * \brief Holds all the data in Nova, providing methods to access the various pieces of data
     *
     * This is also the class that the Minecraft interface goes through to send data to the Nova Renderer
     */
    class data_model : public iconfig_listener {
    public:
        data_model();

        shaderpack &get_loaded_shaderpack();

        texture_manager &get_texture_manager();

        settings &get_render_settings();

        mesh_builder &get_mesh_builder();

        /* From iconfig_listener */

        void on_config_loaded(nlohmann::json &config);

        void on_config_change(nlohmann::json &config);

    private:
        settings render_settings;
        std::unique_ptr<shaderpack> loaded_shaderpack;
        texture_manager textures;
        std::string loaded_shaderpack_name;
        mesh_builder meshes;

    };
}

#endif //RENDERER_MODEL_H
