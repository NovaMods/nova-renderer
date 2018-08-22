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

    nova_renderer::nova_renderer() : render_settings("config/nova/config.json"), context(window) {
        initialize_vulkan_backend();
    }

    void nova_renderer::initialize_vulkan_backend() {

    }

    settings &nova_renderer::get_settings() const {
        return render_settings;
    }

    void nova_renderer::execute_frame() {

    }

    void nova_renderer::load_shaderpack(const std::string &shaderpack_name) {
        const auto shaderpack_data = load_shaderpack_data(fs::path(shaderpack_name));
    }
}
