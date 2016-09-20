/*!
 * \author David
 * \date 23-Jun-16.
 */

#include "settings.h"
#include <easylogging++.h>

#include "utils/utils.h"

namespace nova {
    namespace model {

        settings::settings(std::string filename) {
            LOG(INFO) << "Loading config from " << filename;

            std::ifstream config_file(filename);
            if(config_file.is_open()) {
                std::string buf;
                std::string accum;

                while(getline(config_file, buf)) {
                    accum += buf;
                }

                options = nlohmann::json::parse(accum.c_str());
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
}
