/*!
 * \brief
 *
 * \author ddubois 
 * \date 20-Sep-16.
 */

#include <algorithm>
#include <utility>
#include "shaderpack.h"
#include "../renderpass.h"

#include <easylogging++.h>

namespace nova {
    shaderpack::shaderpack(const std::string &name, std::vector<shader_definition>& shaders, std::shared_ptr<renderpass> our_renderpass) {
        this->name = std::move(name);

        for(auto& shader : shaders) {
            LOG(TRACE) << "Adding shader " << shader.name;
            try {
                loaded_shaders.emplace(shader.name, gl_shader_program(shader, our_renderpass->get_renderpass()));
            } catch(std::exception& e) {
                LOG(ERROR) << "Could not load shader " << shader.name << " because " << e.what();
            }
        }

        LOG(TRACE) << "Shaderpack created";
    }

    gl_shader_program &shaderpack::operator[](std::string key) {
        return get_shader(std::move(key));
    }

    std::unordered_map<std::string, gl_shader_program> &shaderpack::get_loaded_shaders() {
        return loaded_shaders;
    }

    shaderpack& shaderpack::operator=(const shaderpack &other) {
        loaded_shaders = other.loaded_shaders;

        return *this;
    }

    std::string &shaderpack::get_name() {
        return name;
    }

    gl_shader_program &shaderpack::get_shader(std::string key) {
        return loaded_shaders[key];
    }
}
