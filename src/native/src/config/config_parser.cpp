/*!
 * \author David
 * \date 21-Jun-16.
 */

#include <easylogging++.h>
#include <rapidjson/document.h>
#include "config_parser.h"

config_parser::config_parser(std::string filename) {
    LOG(INFO) << "Loading config from " << filename;

    std::ifstream config_file(filename);
    if(config_file.is_open()) {
        std::string buf;
        std::string accum;

        while(getline(config_file, buf)) {
            accum += buf;
        }

        rapidjson::Document config;
        config.Parse(accum.c_str());

        LOG(INFO) << "lastShaderpack: " << config["lastShaderpack"].GetString();
    }

    // TODO: There's going to be a lot more to do when I introduce shader renaming and custon execution orders.
}

