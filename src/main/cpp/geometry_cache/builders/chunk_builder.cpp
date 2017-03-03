/*!
 * \brief
 *
 * \author ddubois 
 * \date 02-Mar-17.
 */

#include "chunk_builder.h"

namespace nova {
    std::unordered_map<std::string, optional<render_object>> get_renderables_from_chunk(mc_chunk chunk, shaderpack& shaders) {
        // Step 1: figure out which blocks are rendered by which shader
        auto& all_shaders = shaders.get_loaded_shaders();
        auto final_geometry = std::unordered_map<std::string, optional<render_object>>{};

        for(auto& shader_entry : all_shaders) {
            final_geometry[shader_entry.first] = build_render_object_for_shader(chunk, shader_entry.second.get_filter());
        }

        return final_geometry;
    }

    optional<render_object> &build_render_object_for_shader(mc_chunk chunk, geometry_filter filter) {
        return <#initializer#>;
    }
}
