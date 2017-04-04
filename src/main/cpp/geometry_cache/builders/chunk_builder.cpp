/*!
 * \brief
 *
 * \author ddubois 
 * \date 02-Mar-17.
 */

#include "chunk_builder.h"

namespace nova {
    std::unordered_map<std::string, optional<render_object>> get_renderables_from_chunk(const mc_chunk& chunk, shaderpack& shaders) {
        // Step 1: figure out which blocks are rendered by which shader
        auto& all_shaders = shaders.get_loaded_shaders();
        auto final_geometry = std::unordered_map<std::string, optional<render_object>>{};

        for(auto& shader_entry : all_shaders) {
            final_geometry[shader_entry.first] = build_render_object_for_shader(chunk, shader_entry.second.get_filter());
        }

        return final_geometry;
    }

    optional<render_object> build_render_object_for_shader(const mc_chunk& chunk, const geometry_filter& filter) {
        auto blocks_that_match_filter = get_blocks_that_match_filter(chunk, filter);

        if(blocks_that_match_filter.size() == 0) {
            return optional<render_object>();
        }

        auto block_mesh_definition = make_mesh_for_blocks(blocks_that_match_filter, chunk);

        auto block_render_object = render_object{};
        block_render_object.geometry = std::make_unique<gl_mesh>(block_mesh_definition);

        return make_optional(std::move(block_render_object));
    }

    std::vector<glm::ivec3> get_blocks_that_match_filter(const mc_chunk &chunk, const geometry_filter &filter) {
        auto blocks_that_match_filter = std::vector<glm::ivec3>{};
        for(int z = 0; z < CHUNK_WIDTH; z++) {
            for(int y = 0; y < CHUNK_HEIGHT; y++) {
                for(int x = 0; x < CHUNK_DEPTH; x++) {
                    int i = x + y * CHUNK_WIDTH + z * CHUNK_WIDTH * CHUNK_HEIGHT;
                    if(filter.matches(chunk.blocks[i])) {
                        blocks_that_match_filter.push_back(glm::ivec3(x, y, z));
                    }
                }
            }
        }
        return blocks_that_match_filter;
    }

    mesh_definition make_mesh_for_blocks(const std::vector<glm::ivec3>& blocks, const mc_chunk& chunk) {
        auto vertices = std::vector<block_vertex>{};
        auto indices = std::vector<unsigned short>{};

        for(auto block_pos : blocks) {
            auto should_make_top_face = get_if_block_at_pos_is_opaque(block_pos + glm::ivec3(0, 1, 0), chunk);
            auto should_make_bottom_face = get_if_block_at_pos_is_opaque(block_pos + glm::ivec3(0, -1, 0), chunk);
            auto should_make_right_face = get_if_block_at_pos_is_opaque(block_pos + glm::ivec3(1, 0, 0), chunk);
            auto should_make_left_face = get_if_block_at_pos_is_opaque(block_pos + glm::ivec3(-1, 0, 0), chunk);
            auto should_make_front_face = get_if_block_at_pos_is_opaque(block_pos + glm::ivec3(0, 0, 1), chunk);
            auto should_make_back_face = get_if_block_at_pos_is_opaque(block_pos + glm::ivec3(0, 0, -1), chunk);
        }
        return mesh_definition{};
    }

    bool get_if_block_at_pos_is_opaque(glm::ivec3 block_pos, const mc_chunk &chunk) {
        // A separate check for each direction to increase code readability and debuggability
        if(block_pos.x < 0 || block_pos.x > CHUNK_WIDTH) {
            return true;
        }

        if(block_pos.y < 0 || block_pos.y > CHUNK_HEIGHT) {
            return true;
        }

        if(block_pos.z < 0 || block_pos.z > CHUNK_DEPTH) {
            return true;
        }

        auto block_idx = pos_to_idx(block_pos);
        auto block = chunk.blocks[block_idx];

        return block.is_transparent();
    }

    int pos_to_idx(const glm::ivec3& pos) {
        return pos.x + pos.y * CHUNK_WIDTH + pos.z * CHUNK_WIDTH * CHUNK_HEIGHT;
    }
}
