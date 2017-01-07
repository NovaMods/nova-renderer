/*!
 * \brief
 *
 * \author ddubois 
 * \date 20-Sep-16.
 */

#include <algorithm>
#include "shaderpack.h"

namespace nova {
    shaderpack::shaderpack(nlohmann::json shaders_json, std::vector<shader_definition> &shaders) {
        // This is gross, but CLion keeps yelling at me
        filter_modifying_functions["solid"] = accept_solid;
        filter_modifying_functions["not_solid"] = reject_solid;
        filter_modifying_functions["transparent"] = accept_transparent;
        filter_modifying_functions["not_transparent"] = reject_transparent;
        filter_modifying_functions["cutout"] = accept_cutout;
        filter_modifying_functions["not_cutout"] = reject_cutout;
        filter_modifying_functions["emissive"] = accept_emissive;
        filter_modifying_functions["not_emissive"] = reject_emissive;
        filter_modifying_functions["damaged"] = accept_damaged;
        filter_modifying_functions["not_damaged"] = reject_damaged;
        filter_modifying_functions["everything_else"] = accept_everything_else;
        filter_modifying_functions["nothing_else"] = reject_everything_else;
    }

    shaderpack::shaderpack(shaderpack &&other) {}

    gl_shader_program &shaderpack::operator[](std::string key) {
        return loaded_shaders[key];
    }

    std::unordered_map<std::string, gl_shader_program> &shaderpack::get_loaded_shaders() {
        return loaded_shaders;
    }
}
