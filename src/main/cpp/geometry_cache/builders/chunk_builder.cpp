/*!
 * \brief Provides definitions for all the functions needed to build geometry for a chunk
 *
 * \author ddubois 
 * \date 02-Mar-17.
 */

#include "chunk_builder.h"
#include "../../utils/utils.h"
#include "../../utils/io.h"
#include "../../render/nova_renderer.h"

#include <iostream>
#include <easylogging++.h>
#include <string.h>

namespace nova {
    std::vector<glm::ivec3> chunk_builder::get_blocks_that_match_filter(const mc_chunk &chunk, const std::shared_ptr<igeometry_filter> filter) {
        auto blocks_that_match_filter = std::vector<glm::ivec3>{};
        for(int z = 0; z < CHUNK_WIDTH; z++) {
            for(int y = 0; y < CHUNK_HEIGHT; y++) {
                for(int x = 0; x < CHUNK_DEPTH; x++) {
                    int i = x + y * CHUNK_WIDTH + z * CHUNK_WIDTH * CHUNK_HEIGHT;
                    auto cur_block = chunk.blocks[i];
                    auto cur_block_definition = block_definitions[cur_block.id];
                    if(filter->matches(cur_block_definition)) {
                        auto pos = glm::ivec3(x, y, z);
                        blocks_that_match_filter.push_back(pos);
                    }
                }
            }
        }
        return blocks_that_match_filter;
    }

    mesh_definition chunk_builder::make_mesh_for_blocks(const std::vector<glm::ivec3>& blocks, const mc_chunk& chunk) {
        auto mesh = mesh_definition{};
        mesh.vertex_format = format::POS_UV_LIGHTMAPUV_NORMAL_TANGENT;

		auto vertices = std::vector<float>{};
		auto indices = std::vector<unsigned int>{};
		auto cur_index = 0;

        for(const auto& block_pos : blocks) {
            auto block_offset = glm::vec3{block_pos};
            auto block_idx = pos_to_idx(block_pos);
            auto& block = block_definitions[chunk.blocks[block_idx].id];

			// Get the geometry for the block
			std::vector<block_face> faces_for_block;
            auto block_state = chunk.blocks[block_idx].state;
            LOG(TRACE) << "Building block with state " << (block_state == nullptr ? "<null state>" : block_state);
            auto state = std::string(block_state);
            auto& model = block_models[state];
            faces_for_block = make_geometry_for_block(block_pos, chunk, model);

			// Put the geometry into our buffer
			for(auto& face : faces_for_block) {
				for(int vert_idx = 0; vert_idx < 4; vert_idx ++) {
                    face.vertices[vert_idx].position += block_offset;
					vertices.insert(vertices.end(), &face.vertices[vert_idx].position.x, &face.vertices[vert_idx].position.x + (sizeof(block_vertex) / sizeof(float)));
				}
				indices.push_back(0 + cur_index);
				indices.push_back(1 + cur_index);
				indices.push_back(2 + cur_index);
				indices.push_back(1 + cur_index);
				indices.push_back(2 + cur_index);
				indices.push_back(3 + cur_index);

				cur_index += 4;
			}
        }

		mesh.vertex_data = vertices;
		mesh.indices = indices;
        mesh.position = {chunk.x, 0, chunk.z};

		return mesh;
    }

	std::vector<block_face> chunk_builder::make_geometry_for_block(const glm::ivec3& block_pos, const mc_chunk& chunk, baked_model& model) {
        auto faces_to_make = std::vector<face_id>{};
        if(!block_at_pos_is_opaque(block_pos + glm::ivec3(0, 1, 0), chunk) &&
           !block_at_offset_is_same(block_pos, glm::ivec3(0, 1, 0), chunk)) {
            faces_to_make.push_back(face_id::TOP);
            LOG(TRACE) << "Making top face";
        }
        if(!block_at_pos_is_opaque(block_pos + glm::ivec3(0, -1, 0), chunk) &&
           !block_at_offset_is_same(block_pos, glm::ivec3(0, -1, 0), chunk)) {
            faces_to_make.push_back(face_id::BOTTOM);
            LOG(TRACE) << "Making bottom face";
        }
        if(!block_at_pos_is_opaque(block_pos + glm::ivec3(1, 0, 0), chunk) &&
           !block_at_offset_is_same(block_pos, glm::ivec3(1, 0, 0), chunk)) {
            faces_to_make.push_back(face_id::RIGHT);
            LOG(TRACE) << "Making right face";
        }
        if(!block_at_pos_is_opaque(block_pos + glm::ivec3(-1, 0, 0), chunk) &&
           !block_at_offset_is_same(block_pos, glm::ivec3(-1, 0, 0), chunk)) {
            faces_to_make.push_back(face_id::LEFT);
            LOG(TRACE) << "Making left face";
        }
        if(!block_at_pos_is_opaque(block_pos + glm::ivec3(0, 0, 1), chunk) &&
           !block_at_offset_is_same(block_pos, glm::ivec3(0, 0, 1), chunk)) {
            faces_to_make.push_back(face_id::FRONT);
            LOG(TRACE) << "Making front face";
        }
        if(!block_at_pos_is_opaque(block_pos + glm::ivec3(0, 0, -1), chunk) &&
           !block_at_offset_is_same(block_pos, glm::ivec3(0, 0, -1), chunk)) {
            faces_to_make.push_back(face_id::BACK);
            LOG(TRACE) << "Making back face";
        }

        auto quads = std::vector<block_face>{};
        for(auto &face : faces_to_make) {
            auto ao = get_ao_in_direction(block_pos, face, chunk);
            quads.insert(quads.end(), model.faces[face].begin(), model.faces[face].end());
        }

        return quads;
    }

	bool chunk_builder::is_cube(const glm::ivec3 pos, const mc_chunk& chunk) {
		return true;
	}

	float chunk_builder::get_ao_in_direction(const glm::vec3 position, const face_id face_to_check, const mc_chunk& chunk) {
		return 0;
	}

    std::unordered_map<int, mc_block_definition>& chunk_builder::get_block_definitions() {
        return block_definitions;
    };

    void chunk_builder::register_block_model(std::string state, int num_quads, mc_baked_quad quads[]) {
        LOG(DEBUG) << "Registering block model for " << state << " with " << num_quads << " quads";
        baked_model model;
        for(int i = 0; i < num_quads; i++) {
            LOG(TRACE) << "Checking quad at " << i;
            auto& quad = quads[i];

            LOG(TRACE) << "About to decode block vertices";
            std::vector<block_vertex> quad_vertices = decode_block_vertices((int*)quad.vertex_data, quad.num_vertices);

            auto face = block_face{};
            std::memcpy(face.vertices, quad_vertices.data(), 28);

            if(is_in_xy_plane(quad_vertices)) {
                // Check if z == 0 or z == 1 to determine if this face is at the edge, and what edge it is
                if(is_at_max_z(quad_vertices)) {
                    model.faces[face_id::FRONT].push_back(face);

                } else if(is_at_min_z(quad_vertices)) {
                    model.faces[face_id::BACK].push_back(face);

                } else {
                    model.faces[face_id::INSIDE_BLOCK].push_back(face);
                }

            } else if(is_in_xz_plane(quad_vertices)) {
                // Check if y == 0 or y == 1 to determine if this face is at the edge, and what edge it is
                if(is_at_max_y(quad_vertices)) {
                    model.faces[face_id::TOP].push_back(face);

                } else if(is_at_min_y(quad_vertices)) {
                    model.faces[face_id::BOTTOM].push_back(face);

                } else {
                    model.faces[face_id::INSIDE_BLOCK].push_back(face);
                }

            } else if(is_in_yz_plane(quad_vertices)) {
                if(is_at_max_x(quad_vertices)) {
                    model.faces[face_id::RIGHT].push_back(face);

                } else if(is_at_min_x(quad_vertices)) {
                    model.faces[face_id::LEFT].push_back(face);

                } else {
                    model.faces[face_id::INSIDE_BLOCK].push_back(face);
                }

            } else {
                model.faces[face_id::INSIDE_BLOCK].push_back(face);
            }
        }

        block_models[state] = model;
    }

    bool chunk_builder::block_at_pos_is_opaque(glm::ivec3 block_pos, const mc_chunk& chunk) {
        // A separate check for each direction to increase code readability and debuggability
        if(block_pos.x < 0 || block_pos.x >= CHUNK_WIDTH) {
            return false;
        }

        if(block_pos.y < 0 || block_pos.y >= CHUNK_HEIGHT) {
            return false;
        }

        if(block_pos.z < 0 || block_pos.z >= CHUNK_DEPTH) {
            return false;
        }

        auto block_idx = pos_to_idx(block_pos);
        auto block = chunk.blocks[block_idx];

        return !block_definitions[block.id].is_transparent();
    }

    bool chunk_builder::block_at_offset_is_same(glm::ivec3 block_pos, glm::ivec3 offset, const mc_chunk& chunk) {
        LOG(TRACE) << "Checking if block at offset " << offset << " is the same";
        // A separate check for each direction to increase code readability and debuggability
        if(block_pos.x+offset.x < 0 || block_pos.x+offset.x >= CHUNK_WIDTH) {
            return false;
        }

        if(block_pos.y+offset.y < 0 || block_pos.y+offset.y >= CHUNK_HEIGHT) {
            return false;
        }

        if(block_pos.z+offset.z < 0 || block_pos.z+offset.z >= CHUNK_DEPTH) {
            return false;
        }

        LOG(TRACE) << "Block at pos " << block_pos + offset << " is in the chunk";

        auto block_idx = pos_to_idx(block_pos);
        auto block = block_definitions[chunk.blocks[block_idx].id];
        auto block_idx2 = pos_to_idx(block_pos+offset);
        auto block2 = block_definitions[chunk.blocks[block_idx2].id];

        return strcmp(block.name, block2.name) == 0;
    }


    int pos_to_idx(const glm::ivec3& pos) {
        return pos.x + pos.y * CHUNK_WIDTH + pos.z * CHUNK_WIDTH * CHUNK_HEIGHT;
    }


    bool is_in_plane(const std::vector<block_vertex>& vertices, int plane) {
        auto compare_val = vertices[0].position[plane];

        for(const auto& vertex : vertices) {
            if(vertex.position[plane] != compare_val) {
                return false;
            }
        }

        return true;
    }

    bool is_in_xy_plane(const std::vector<block_vertex>& vertices) {
        return is_in_plane(vertices, 2);
    }

    bool is_in_xz_plane(const std::vector<block_vertex>& vertices) {
        return is_in_plane(vertices, 1);
    }

    bool is_in_yz_plane(const std::vector<block_vertex>& vertices) {
        return is_in_plane(vertices, 0);
    }


    bool index_is_at_value(const std::vector<block_vertex> &vertices, int index, float val);

    bool is_at_max_x(const std::vector<block_vertex>& vertices) {
        return index_is_at_value(vertices, 0, 1);
    }

    bool is_at_min_x(const std::vector<block_vertex>& vertices) {
        return index_is_at_value(vertices, 0, 0);
    }

    bool is_at_max_y(const std::vector<block_vertex>& vertices) {
        return index_is_at_value(vertices, 1, 1);
    }

    bool is_at_min_y(const std::vector<block_vertex>& vertices) {
        return index_is_at_value(vertices, 1, 0);
    }

    bool is_at_max_z(const std::vector<block_vertex>& vertices) {
        return index_is_at_value(vertices, 2, 1);
    }

    bool is_at_min_z(const std::vector<block_vertex>& vertices) {
        return index_is_at_value(vertices, 2, 0);
    }

    bool index_is_at_value(const std::vector<block_vertex> &vertices, int index, float val) {
        for(const auto& vertex : vertices) {
            if(vertex.position[index] != val) {
                return false;
            }
        }

        return true;
    }

    el::base::Writer &operator<<(el::base::Writer &out, const block_vertex& vert) {
        out << "block_vertex { position=" << vert.position << ", uv=" << vert.uv << ", lightmap_uv="
            << vert.lightmap_uv << ", normal=" << vert.normal << ", tangent=" << vert.tangent << "}";

        return out;
    }
}
