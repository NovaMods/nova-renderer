/*! 
 * \author gold1 
 * \date 13-Jun-17.
 */

#include "shader_source_structs.h"
#include <easylogging++.h>

namespace nova {
    shader_definition::shader_definition(nlohmann::json& json)  {
        name = json["name"];

        LOG(DEBUG) << "Creating shader definition from json " << json;

        filter_expression = json["filters"];

        if(json.find("fallback") != json.end()) {
            std::string fallback_name_str = json["fallback"];
            fallback_name = optional<std::string>(fallback_name_str);
        }
    }

    el::base::Writer& operator<<(el::base::Writer& out, const std::vector<shader_line>& lines) {
        for(const auto& line : lines) {
            out << line;
        }

        return out;
    }

    el::base::Writer& operator<<(el::base::Writer& out, const shader_line& line) {
        out << "\t" << line.line_num << "(" << line.shader_name << ") " << line.line << "\n";
        return out;
    }
}