/*!
 * \author David
 * \date 23-Jun-16.
 */

#include "nova_settings.hpp"
#include "../util/utils.hpp"

namespace nova {
    void nova_settings::register_change_listener(iconfig_listener *new_listener) {
        config_change_listeners.push_back(new_listener);
    }

    settings_options &nova_settings::get_options() {
        return options;
    }

    settings_options nova_settings::get_options() const {
        return options;
    }

    void nova_settings::update_config_changed() {
        for(iconfig_listener *l : config_change_listeners) {
            l->on_config_change(options);
        }
    }

    void nova_settings::update_config_loaded() {
        for(iconfig_listener *l : config_change_listeners) {
            l->on_config_loaded(options);
        }
    }
}  // namespace nova
