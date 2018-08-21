/*!
 * \author David
 * \date 23-Jun-16.
 */

#include "settings.hpp"
#include "util/utils.hpp"

#include <easylogging++.h>

namespace nova {
    settings::settings(std::string filename) {
        LOG(INFO) << "Loading config from " << filename;

        std::ifstream config_file(filename);
        if(config_file.is_open()) {
            options = load_json_from_stream(config_file);
        }
    }

    void settings::register_change_listener(iconfig_listener *new_listener) {
        config_change_listeners.push_back(new_listener);
    }

    nlohmann::json &settings::get_options() {
        return options;
    }

    void settings::update_config_changed() {
        for(iconfig_listener *l : config_change_listeners) {
            l->on_config_change(options["settings"]);
        }
    }

    void settings::update_config_loaded() {
        for(iconfig_listener *l : config_change_listeners) {
            l->on_config_loaded(options["readOnly"]);
        }
    }
}
