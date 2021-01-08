#include "nova_renderer/nova_settings.hpp"

namespace nova::renderer {
    NovaSettingsAccessManager::NovaSettingsAccessManager(NovaSettings settings) : settings(std::move(settings)) {}

    void NovaSettingsAccessManager::register_change_listener(ConfigListener* new_listener) {
        config_change_listeners.push_back(new_listener);
    }

    void NovaSettingsAccessManager::update_config_changed() {
        config_change_listeners.each_fwd([&](ConfigListener* l) { l->on_config_change(*this); });
    }

    void NovaSettingsAccessManager::update_config_loaded() {
        config_change_listeners.each_fwd([&](ConfigListener* l) { l->on_config_loaded(*this); });
    }

    const NovaSettings* NovaSettingsAccessManager::operator->() const { return &settings; }
} // namespace nova::renderer
