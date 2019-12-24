#include "nova_renderer/nova_settings.hpp"

#include <memory_resource>
#include <utility>

namespace nova::renderer {
    NovaSettingsAccessManager::NovaSettingsAccessManager(NovaSettings settings) : settings(std::move(settings)) {}

    void NovaSettingsAccessManager::register_change_listener(ConfigListener* new_listener) {
        config_change_listeners.push_back(new_listener);
    }

    void NovaSettingsAccessManager::update_config_changed() {
        for(ConfigListener* l : config_change_listeners) {
            l->on_config_change(*this);
        }
    }

    void NovaSettingsAccessManager::update_config_loaded() {
        for(ConfigListener* l : config_change_listeners) {
            l->on_config_loaded(*this);
        }
    }

    const NovaSettings* NovaSettingsAccessManager::operator->() const { return &settings; }
} // namespace nova::renderer
