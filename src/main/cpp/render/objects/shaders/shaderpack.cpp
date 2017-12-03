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
    shaderpack::shaderpack(const std::string &name, std::vector<std::pair<material_state, shader_definition>>& shaders, const vk::RenderPass our_renderpass) :
            name(name), device(render_context::instance.device) {

        create_pipeline_cache();

        for(auto& shader : shaders) {
            LOG(TRACE) << "Adding shader " << shader.second.name;
            try {
                loaded_shaders.emplace(shader.second.name, gl_shader_program(shader.second, shader.first, our_renderpass, pipeline_cache));
            } catch(std::exception& e) {
                LOG(ERROR) << "Could not load shader " << shader.second.name << " because " << e.what();
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

    void shaderpack::create_pipeline_cache() {
        vk::PipelineCacheCreateInfo cache_create_info = {};

        // TODO: Store a pipeline for each shaderpack on disk, only creating a new cache when the shaderpack has changed
        // But for now I won't do that cause I really wanna see stuff again

        pipeline_cache = device.createPipelineCache(cache_create_info);
    }
}
