/*!
 * \author ddubois 
 * \date 14-Aug-18.
 */

#include "nova_renderer.hpp"

namespace nova {
    std::unique_ptr<nova_renderer> nova_renderer::instance;

    nova_renderer *nova_renderer::initialize(std::shared_ptr<render_engine> engine, settings_options &settings) {
        if(!instance) {
            instance = std::make_unique<nova_renderer>(engine, settings);
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

    nova_renderer::nova_renderer(std::shared_ptr<render_engine> engine, settings_options &settings) : render_settings(settings), engine(engine) {
        engine->init(render_settings);
    }

    settings &nova_renderer::get_settings() {
        return render_settings;
    }
}
