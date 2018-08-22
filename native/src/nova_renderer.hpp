/*!
 * \brief Main class for Nova. This class exists as a singleton so it's always available
 *
 * \author ddubois 
 * \date 14-Aug-18.
 */

#ifndef NOVA_RENDERER_NOVA_RENDERER_H
#define NOVA_RENDERER_NOVA_RENDERER_H

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

#include "vulkan/vulkan_render_device.hpp"
#include "vulkan/glfw_window.hpp"
#include "input/input_handler.hpp"
#include "settings.hpp"

namespace nova {
    class texture_store;
    class mesh_store;

    /*!
     * \brief Main class for Nova. Owns all of Nova's resources and provides a way to access them
     */
    class nova_renderer {
    public:
        /*!
         * \brief Initializes the Nova Renderer
         *
         * Instantiates an instance of the Nova Renderer and saves it to the singleton instance. If Nova has already
         * been initialized, nothing is done
         */
        static void initialize();

        /*!
         * \brief Retrieves the Nova Renderer singleton. If the singleton hasn't been initialized, it get initialized
         * \return The Nova Renderer singleton
         */
        static nova_renderer* get_instance();

        /*!
         * \brief Deinitializes the Nova Renderer
         */
        static void deinitialize();

        /*!
         * \brief Initializes the Nova Renderer
         */
        nova_renderer();

        /*!
         * \brief Loads the shaderpack with the given name
         *
         * This method will first try to load from the `shaderpacks/` folder (mimicing Optifine shaders). If the
         * shaderpack isn't found there, it'll try to load it from the `resourcepacks/` directory (mimicing Bedrock
         * shaders). If the shader can't be found at either place, a `nova::resource_not_found` exception will be thrown
         *
         * \param shaderpack_name The name of the shaderpack to load
         */
        void load_shaderpack(const std::string& shaderpack_name);

        /*!
         * \brief Executes a single frame
         */
        void execute_frame();

        /*!
         * \brief Returns the texture store where all the textures are stored
         *
         * \return Nova's texture store
         */
        texture_store& get_texture_store() const;

        /*!
         * \brief Returns the mesh store, where Nova stores all its meshes
         *
         * \return Nova's mesh store
         */
        mesh_store& get_mesh_store() const;

        input_handler& get_input_handler() const;

        settings& get_settings() const;

    private:
        static std::unique_ptr<nova_renderer> instance;

        settings render_settings;

#ifdef SUPPORT_VULKAN
        glfw_vk_window window;

        vulkan_render_context context;

        void initialize_vulkan_backend();
#endif
    };
}

#endif //NOVA_RENDERER_NOVA_RENDERER_H
