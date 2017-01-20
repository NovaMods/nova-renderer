/*!
 * \brief
 *
 * \author ddubois 
 * \date 20-Sep-16.
 */

#include <algorithm>
#include "shaderpack.h"

namespace nova {
    shaderpack::shaderpack(std::string name, nlohmann::json shaders_json, std::vector<shader_definition> &shaders) {

        for(auto& shader : shaders) {
            loaded_shaders.emplace(shader.name, gl_shader_program(shader));
        }
    }

    gl_shader_program &shaderpack::operator[](std::string key) {
        return loaded_shaders[key];
    }

    std::unordered_map<std::string, gl_shader_program> &shaderpack::get_loaded_shaders() {
        return loaded_shaders;
    }

    void shaderpack::operator=(const shaderpack &other) {
        loaded_shaders = other.loaded_shaders;
    }

    std::string &shaderpack::get_name() {
        return name;
    }
}
