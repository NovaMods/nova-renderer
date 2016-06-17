/*!
 * \author David
 * \date 23-Jun-16.
 */

#include "config.h"
#include <easylogging++.h>

#include "utils/utils.h"

config::config(std::string filename) {
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

void config::register_change_listener(iconfig_change_listener *new_listener) {
    config_change_listeners.push_back(new_listener);
}

nlohmann::json & config::get_options() {
    return options;
}

void config::update_change_listeners() {
    for(iconfig_change_listener * l : config_change_listeners) {
        l->on_config_change(options);
    }
}
