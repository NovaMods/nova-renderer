/*! 
 * \author gold1 
 * \date 13-Jun-17.
 */

#include "shader_source_structs.h"
#include "../../render/objects/renderpasses/materials.h"
#include <easylogging++.h>

namespace nova {
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

    std::vector<std::string> spirv_extensions = {
            ".vert.spirv",
            ".geom.spirv",
            ".tesc.spirv",
            ".tese.spirv",
            ".frag.spirv",

            ".vsh.spirv",
            ".geo.spirv",
            ".tsc.spirv",
            ".tse.spirv",
            ".fsh.spirv",

            ".vertex.spirv",
            ".geometry.spirv",
            ".tess_control.spirv",
            ".tess_eval.spirv",
            ".fragment.spirv",
    };

    std::vector<std::string> glsl_extensions = {
            ".vert",
            ".geom",
            ".tesc",
            ".tese",
            ".frag",

            ".vsh",
            ".geo",
            ".tsc",
            ".tse",
            ".fsh"
    };

    std::vector<std::string> glsl_es_extensions = {
            ".vertex",
            ".geometry",
            ".tess_control",
            ".tess_eval",
            ".fragment",
    };

    std::vector<std::string> hlsl_extensions = {
            ".vert.hlsl",
            ".geom.hlsl",
            ".tesc.hlsl",
            ".tese.hlsl",
            ".frag.hlsl",

            ".vsh.hlsl",
            ".geo.hlsl",
            ".tsc.hlsl",
            ".tse.hlsl",
            ".fsh.hlsl",

            ".vertex.hlsl",
            ".geometry.hlsl",
            ".tess_control.hlsl",
            ".tess_eval.hlsl",
            ".fragment.hlsl",
    };

    shader_langauge_enum language_from_extension(const std::string& extension) {
        if(std::find(spirv_extensions.begin(), spirv_extensions.end(), extension) != spirv_extensions.end()) {
            return shader_langauge_enum::SPIRV;

        } else if(std::find(glsl_extensions.begin(), glsl_extensions.end(), extension) != glsl_extensions.end()) {
            return shader_langauge_enum::GLSL;

        } else if(std::find(glsl_es_extensions.begin(), glsl_es_extensions.end(), extension) != glsl_es_extensions.end()) {
            return shader_langauge_enum::GLSLES;

        } else if(std::find(hlsl_extensions.begin(), hlsl_extensions.end(), extension) != hlsl_extensions.end()) {
            return shader_langauge_enum::HLSL;

        }
    }
}