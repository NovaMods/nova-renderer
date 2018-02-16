/*!
 * \brief
 *
 * \author ddubois 
 * \date 20-Sep-16.
 */

#include <algorithm>
#include <utility>
#include "shaderpack.h"
#include "../../vulkan/render_context.h"
#include "../../nova_renderer.h"

#include <easylogging++.h>

namespace nova {
    shaderpack::shaderpack(const std::string &name, std::vector<std::pair<material_state, shader_definition>>& shaders, const vk::RenderPass our_renderpass, std::shared_ptr<render_context> context, std::shared_ptr<shader_resource_manager> shader_resources) :
            name(name), device(context->device) {
        auto pipeline_cache = context->pipeline_cache;

        for(auto& shader : shaders) {
            auto shader_def = shader.second;
            if(!shader_def.vertex_source.empty() && !shader_def.fragment_source.empty()) {
                LOG(TRACE) << "Adding shader " << shader.second.name;
                try {
                    loaded_shaders.emplace(shader.second.name, vk_shader_program(shader.second, shader.first, our_renderpass, pipeline_cache, device, shader_resources));
                } catch (std::exception &e) {
                    LOG(ERROR) << "Could not load shader " << shader.second.name << " because " << e.what();
                }
            } else {
                LOG(ERROR) << "Missing either vertex or fragment source for shader " << shader.second.name << ". Check earlier in the logs for a better error";
            }
        }

        LOG(TRACE) << "Shaderpack created";
    }

    vk_shader_program &shaderpack::operator[](std::string key) {
        return get_shader(std::move(key));
    }

    std::unordered_map<std::string, vk_shader_program> &shaderpack::get_loaded_shaders() {
        return loaded_shaders;
    }

    std::string &shaderpack::get_name() {
        return name;
    }

    vk_shader_program &shaderpack::get_shader(std::string key) {
        return loaded_shaders[key];
    }
}
