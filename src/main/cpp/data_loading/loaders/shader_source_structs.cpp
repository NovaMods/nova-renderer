/*! 
 * \author gold1 
 * \date 13-Jun-17.
 */

#include "shader_source_structs.h"
#include <easylogging++.h>

namespace nova {
    shader_definition::shader_definition(nlohmann::json& json)  {
        name = json["name"];

        LOG(INFO) << "json: " << json;

        filter_expression = json["filters"];

        if(json.find("fallback") != json.end()) {
            std::string fallback_name_str = json["fallback"];
            fallback_name = optional<std::string>(fallback_name_str);
        }
    }
}