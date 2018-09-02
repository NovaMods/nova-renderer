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
    template <typename RenderEngine>
    class nova_renderer {
    public:
        /*!
         * \brief Initializes the Nova Renderer
         */
        explicit nova_renderer() : render_settings(settings_options{RenderEngine::get_engine_name()}) {
            engine = std::make_unique<RenderEngine>(render_settings);
        };

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

        RenderEngine *get_engine();

        static nova_renderer<RenderEngine> *initialize() {
            return (instance = new nova_renderer<RenderEngine>());
        }

        static nova_renderer<RenderEngine>* get_instance();

        static void deinitialize();

    private:
        settings render_settings;
        std::unique_ptr<RenderEngine> engine;

        static nova_renderer<RenderEngine> *instance;
    };

    template<typename RenderEngine>
    nova_renderer<RenderEngine>* nova_renderer<RenderEngine>::instance;

    template <typename RenderEngine>
    settings &nova_renderer<RenderEngine>::get_settings() {
        return render_settings;
    }

    template <typename RenderEngine>
    void nova_renderer<RenderEngine>::execute_frame() {
        // Transition the swapchain image from being presentable to being writable from a shader
        icommand_buffer* swapchain_image_command_buffer = engine->allocate_command_buffer(command_buffer_type::GENERIC);

        resource_barrier_data swapchain_image_to_shader_writable = {};
        swapchain_image_to_shader_writable.initial_layout = resource_layout::PRESENT;
        swapchain_image_to_shader_writable.final_layout = resource_layout::RENDER_TARGET;
        swapchain_image_command_buffer->resource_barrier({swapchain_image_command_buffer});

        engine->present_swapchain_image();
    }

    template <typename RenderEngine>
    void nova_renderer<RenderEngine>::load_shaderpack(const std::string &shaderpack_name) {
        const auto shaderpack_data = load_shaderpack_data(fs::path(shaderpack_name));
    }

    template<typename RenderEngine>
    RenderEngine *nova_renderer<RenderEngine>::get_engine() {
        return engine.get();
    }

    template <typename RenderEngine>
    nova_renderer<RenderEngine>* nova_renderer<RenderEngine>::get_instance() {
        return instance;
    }

    template <typename RenderEngine>
    void nova_renderer<RenderEngine>::deinitialize() {

    }
}


#endif //NOVA_RENDERER_NOVA_RENDERER_H
