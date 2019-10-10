#include <utility>
#include "nova_renderer/nova_settings.hpp"
#include "nova_renderer/util/utils.hpp"

namespace nova::renderer {
    NovaSettingsAccessManager::NovaSettingsAccessManager(NovaSettings settings) : settings(std::move(settings)) {
    }

    void NovaSettingsAccessManager::register_change_listener(ConfigListener* new_listener) {
        config_change_listeners.push_back(new_listener);
    }

    void NovaSettingsAccessManager::update_config_changed() {
        for(ConfigListener* l : config_change_listeners) {
            l->on_config_change(settings);
        }
    }

    void NovaSettingsAccessManager::update_config_loaded() {
        for(ConfigListener* l : config_change_listeners) {
            l->on_config_loaded(settings);
        }
    }
} // namespace nova::renderer
