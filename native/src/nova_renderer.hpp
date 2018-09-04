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
#include "loading/shaderpack_loading.hpp"

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
         */
        explicit nova_renderer();

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

        settings &get_settings();

        render_engine *get_engine();

        static nova_renderer *initialize() {
            return (instance = new nova_renderer());
        }

        static nova_renderer* get_instance();

        static void deinitialize();

    private:
        settings render_settings;
        std::unique_ptr<render_engine> engine;

        static nova_renderer *instance;
    };
}


#endif //NOVA_RENDERER_NOVA_RENDERER_H
