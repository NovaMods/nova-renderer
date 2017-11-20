/*! 
 * \author gold1 
 * \date 13-Jun-17.
 */

#include "shader_source_structs.h"
#include "../../render/objects/renderpasses/materials.h"
#include <easylogging++.h>

namespace nova {
    shader_definition::shader_definition(const nova::material_state &material) : name(material.name), filter_expression(*material.filters)  {
        if(material.fallback) {
            std::string fallback_name_str = *material.fallback;
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