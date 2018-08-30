/*!
 * \author ddubois 
 * \date 14-Aug-18.
 */

#include "nova_renderer.hpp"
#include "loading/loading_utils.hpp"
#include "loading/shaderpack_loading.hpp"

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

namespace nova {
    std::unique_ptr<nova_renderer> nova_renderer::instance;

    nova_renderer *nova_renderer::initialize(render_engine *engine) {
        if(!instance) {
            instance = std::make_unique<nova_renderer>(engine);
        } else {
            throw already_initialized_exception("Nova has already been initialized");
        }
    }

    nova_renderer *nova_renderer::get_instance() {
        if(!instance) {
            throw uninitialized_exception("Nova has not been initialized yet");
        }

        return instance.get();
    }

    void nova_renderer::deinitialize() {
        if(instance) {
            instance.reset(nullptr);
        }
    }

    nova_renderer::nova_renderer(render_engine *engine) : render_settings(settings_options{engine->get_engine_name()}), engine(engine) {
        engine->init(render_settings);
    }

    settings &nova_renderer::get_settings() {
        return render_settings;
    }

    void nova_renderer::execute_frame() {

    }

    void nova_renderer::load_shaderpack(const std::string &shaderpack_name) {
        const auto shaderpack_data = load_shaderpack_data(fs::path(shaderpack_name));
    }
}
