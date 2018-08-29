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

#include "settings/settings.hpp"
#include "render_engine/render_engine.hpp"
#include "util/macros.hpp"

NOVA_EXCEPTION(nova, already_initialized_exception)
NOVA_EXCEPTION(nova, uninitialized_exception)

namespace nova {
    /*!
     * \brief Main class for Nova. Owns all of Nova's resources and provides a way to access them
     */
    class nova_renderer {
    public:
        /*!
         * \brief Initializes the Nova Renderer
         *
         * Instantiates an instance of the Nova Renderer and saves it to the singleton instance. If Nova has already
         * been initialized, already_initialized_exception is thrown
         *
         * \param engine The render engine to use
         */
        static nova_renderer *initialize(render_engine *engine);

        /*!
         * \brief Retrieves the Nova Renderer singleton. If the singleton hasn't been initialized, an uninitialized_exception is thrown
         * \return The Nova Renderer singleton
         */
        static nova_renderer *get_instance();

        /*!
         * \brief Deinitializes the Nova Renderer
         */
        static void deinitialize();

        /*!
         * \brief Initializes the Nova Renderer
         *
         * \param engine The render engine to use
         */
        nova_renderer(render_engine *engine);

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

        settings &get_settings();

    private:
        static std::unique_ptr<nova_renderer> instance;

        settings render_settings;
        render_engine *engine;
    };
}

#endif //NOVA_RENDERER_NOVA_RENDERER_H
