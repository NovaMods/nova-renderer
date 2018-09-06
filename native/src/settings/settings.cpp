/*!
 * \author David
 * \date 23-Jun-16.
 */

#include "settings.hpp"
#include "../util/utils.hpp"
#include <cpptoml.h>

namespace nova {
    settings::settings(const std::string& filename) {
        const std::shared_ptr<cpptoml::table> &options_toml = cpptoml::parse_file(filename);

        options.debug.enable_renderdoc = options_toml->get_qualified_as<bool>("debug.enableRenderdoc").value_or(false);
        options.cache.loaded_shaderpack = options_toml->get_qualified_as<std::string>("cache.loadedShaderpack").value_or("DefaultShaderpack");
        options.api.vulkan.application_name = options_toml->get_qualified_as<std::string>("api.vulkan.applicationName").value_or("Nova Renderer");
        options.api.vulkan.appliction_version.major = options_toml->get_qualified_as<uint32_t>("api.vulkan.applicationVersion.major").value_or(0);
        options.api.vulkan.appliction_version.minor = options_toml->get_qualified_as<uint32_t>("api.vulkan.applicationVersion.minor").value_or(0);
        options.api.vulkan.appliction_version.patch = options_toml->get_qualified_as<uint32_t>("api.vulkan.applicationVersion.patch").value_or(0);
    }

    void settings::register_change_listener(iconfig_listener *new_listener) {
        config_change_listeners.push_back(new_listener);
    }

    settings_options &settings::get_options() {
        return options;
    }

    settings_options settings::get_options() const {
        return options;
    }

    void settings::update_config_changed() {
        for(iconfig_listener *l : config_change_listeners) {
            l->on_config_change(options);
        }
    }

    void settings::update_config_loaded() {
        for(iconfig_listener *l : config_change_listeners) {
            l->on_config_loaded(options);
        }
    }
}
