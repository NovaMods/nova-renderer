/*!
 * \brief
 *
 * \author ddubois 
 * \date 02-Mar-17.
 */

#include "chunk_builder.h"

namespace nova {
    std::unordered_map<std::string, optional<render_object>> get_renderables_from_chunk(mc_chunk& chunk, shaderpack& shaders) {
        // Step 1: figure out which blocks are rendered by which shader
        auto& all_shaders = shaders.get_loaded_shaders();
        auto final_geometry = std::unordered_map<std::string, optional<render_object>>{};

        for(auto& shader_entry : all_shaders) {
            final_geometry[shader_entry.first] = build_render_object_for_shader(chunk, shader_entry.second.get_filter());
        }

        return final_geometry;
    }

    optional<render_object> build_render_object_for_shader(mc_chunk& chunk, geometry_filter filter) {
        std::vector<int> blocks_that_match_filter = get_blocks_that_match_filter(chunk, filter);

        if(blocks_that_match_filter.size() == 0) {
            return optional<render_object>();
        }

        auto block_mesh_definition = make_mesh_for_blocks(blocks_that_match_filter, chunk);

        auto block_render_object = render_object{};
        block_render_object.geometry = std::make_unique<gl_mesh>(block_mesh_definition);

        return make_optional(block_render_object);
    }

    std::vector<int> get_blocks_that_match_filter(const mc_chunk &chunk, geometry_filter &filter) {
        auto blocks_that_match_filter = std::vector<int>{};
        for(int i = 0; i < 16 * 16 * 256; i++) {
            if(filter.matches(chunk.blocks[i])) {
                blocks_that_match_filter.push_back(i);
            }
        }
        return blocks_that_match_filter;
    }

    mesh_definition make_mesh_for_blocks(std::vector<int> blocks, mc_chunk& chunk) {
        return nullptr;
    }
}
