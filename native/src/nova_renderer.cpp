/*!
 * \author ddubois 
 * \date 14-Aug-18.
 */

#include "nova_renderer.hpp"

namespace nova {
    std::unique_ptr<nova_renderer> nova_renderer::instance;

    void nova_renderer::initialize() {
        if(!instance) {
            instance = std::make_unique<nova_renderer>();
        }
    }

    nova_renderer *nova_renderer::get_instance() {
        if(!instance) {
            initialize();
        }

        return instance.get();
    }

    void nova_renderer::deinitialize() {
        if(instance) {
            instance.release();
        }
    }

    nova_renderer::nova_renderer() {
        initialize_vulkan_backend();
    }

    void nova_renderer::initialize_vulkan_backend() {

    }
}
