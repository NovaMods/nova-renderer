/*!
 * \author David
 * \date 23-Jun-16.
 */

#include "nova_renderer/nova_settings.hpp"
#include "nova_renderer/util/utils.hpp"

namespace nova::renderer {
    void NovaSettings::register_change_listener(ConfigListener* new_listener) { config_change_listeners.push_back(new_listener); }

    void NovaSettings::update_config_changed() {
        for(ConfigListener* l : config_change_listeners) {
            l->on_config_change(*this);
        }
    }

    void NovaSettings::update_config_loaded() {
        for(ConfigListener* l : config_change_listeners) {
            l->on_config_loaded(*this);
        }
    }
} // namespace nova::renderer
