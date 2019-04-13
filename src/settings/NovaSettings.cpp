/*!
 * \author David
 * \date 23-Jun-16.
 */

#include <nova_renderer/nova_settings.hpp>
#include <nova_renderer/util/utils.hpp>

namespace nova::renderer {
    void nova_settings::register_change_listener(IconfigListener* new_listener) { config_change_listeners.push_back(new_listener); }

    void nova_settings::update_config_changed() {
        for(IconfigListener* l : config_change_listeners) {
            l->on_config_change(*this);
        }
    }

    void nova_settings::update_config_loaded() {
        for(IconfigListener* l : config_change_listeners) {
            l->on_config_loaded(*this);
        }
    }
} // namespace nova::renderer
