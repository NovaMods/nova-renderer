/*!
 * \brief Provides definitions for all the functions needed to build geometry for a chunk
 *
 * \author ddubois 
 * \date 02-Mar-17.
 */

#include "chunk_builder.h"
#include "../../utils/utils.h"

#include <easylogging++.h>

namespace nova {
    std::unordered_map<std::string, optional<render_object>> get_renderables_from_chunk(const mc_chunk& chunk, shaderpack& shaders) {
        auto& all_shaders = shaders.get_loaded_shaders();
        auto final_geometry = std::unordered_map<std::string, optional<render_object>>{};

        for(auto& shader_entry : all_shaders) {
            final_geometry[shader_entry.first] = build_render_object_for_shader(chunk, shader_entry.second.get_filter());
        }

        return final_geometry;
    }

    optional<render_object> build_render_object_for_shader(const mc_chunk& chunk, const std::shared_ptr<igeometry_filter> filter) {
        auto blocks_that_match_filter = get_blocks_that_match_filter(chunk, filter);

        if(blocks_that_match_filter.size() == 0) {
            LOG(INFO) << "No blocks match our filter";
            return optional<render_object>();
        }

        auto block_mesh_definition = make_mesh_for_blocks(blocks_that_match_filter, chunk);

        auto block_render_object = render_object{};
        block_render_object.geometry = std::make_unique<gl_mesh>(block_mesh_definition);

        return make_optional(std::move(block_render_object));
    }

    std::vector<glm::ivec3> get_blocks_that_match_filter(const mc_chunk &chunk, const std::shared_ptr<igeometry_filter> filter) {
        auto blocks_that_match_filter = std::vector<glm::ivec3>{};
        for(int z = 0; z < CHUNK_WIDTH; z++) {
            for(int y = 0; y < CHUNK_HEIGHT; y++) {
                for(int x = 0; x < CHUNK_DEPTH; x++) {
                    int i = x + y * CHUNK_WIDTH + z * CHUNK_WIDTH * CHUNK_HEIGHT;
                    auto cur_block = chunk.blocks[i];
                    if(filter->matches(cur_block)) {
                        LOG(INFO) << "Adding block " << cur_block.name;
                        auto pos = glm::ivec3(x, y, z);
                        blocks_that_match_filter.push_back(pos);
                    } else {
                        LOG(INFO) << "Block " << cur_block.name << " does not match filter " << filter->to_string();
                    }
                }
            }
        }
        return blocks_that_match_filter;
    }

    mesh_definition make_mesh_for_blocks(const std::vector<glm::ivec3>& blocks, const mc_chunk& chunk) {
        auto mesh = mesh_definition{};
        mesh.vertex_format = format::POS_UV_LIGHTMAPUV_NORMAL_TANGENT;

		auto vertices = std::vector<float>{};
		auto indices = std::vector<unsigned int>{};
		auto cur_index = 0;

        for(const auto& block_pos : blocks) {
            auto block_offset = glm::vec3{block_pos};

			// Get the geometry for the block
			std::vector<block_face> faces_for_block;
			if(is_cube(block_pos, chunk)) {
				faces_for_block = make_geometry_for_block(block_pos, chunk);
			} else {
				// Use the block model registry
				LOG(WARNING) << "Block models not implemented. Fix it.";
			}

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

		return mesh;
    }

	std::vector<block_face> make_geometry_for_block(const glm::ivec3& block_pos, const mc_chunk& chunk) {
		auto faces_to_make = std::vector<face_id>{};
		if(!block_at_pos_is_opaque(block_pos + glm::ivec3(0, 1, 0), chunk)) {
            faces_to_make.push_back(face_id::TOP);
        }
		if(!block_at_pos_is_opaque(block_pos + glm::ivec3(0, -1, 0), chunk)) {
            faces_to_make.push_back(face_id::BOTTOM);
        }
		if(!block_at_pos_is_opaque(block_pos + glm::ivec3(1, 0, 0), chunk)) {
            faces_to_make.push_back(face_id::RIGHT);
        }
		if(!block_at_pos_is_opaque(block_pos + glm::ivec3(-1, 0, 0), chunk)) {
            faces_to_make.push_back(face_id::LEFT);
        }
		if(!block_at_pos_is_opaque(block_pos + glm::ivec3(0, 0, 1), chunk)) {
            faces_to_make.push_back(face_id::FRONT);
        }
		if(!block_at_pos_is_opaque(block_pos + glm::ivec3(0, 0, -1), chunk)) {
            faces_to_make.push_back(face_id::BACK);
        }

		auto quads = std::vector<block_face>{};
		for(auto& face : faces_to_make) {
			auto ao = get_ao_in_direction(block_pos, face, chunk);
			quads.push_back(make_quad(face, 1));
		}

		return quads;
	}

    bool block_at_pos_is_opaque(glm::ivec3 block_pos, const mc_chunk& chunk) {
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

        return block.is_opaque;

    }

    int pos_to_idx(const glm::ivec3& pos) {
        return pos.x + pos.y * CHUNK_WIDTH + pos.z * CHUNK_WIDTH * CHUNK_HEIGHT;
    }

    block_face make_quad(const face_id which_face, const float size) {
        glm::vec3 positions[4];
        glm::vec3 normal;
        glm::vec3 tangent;
        if(which_face == face_id::LEFT) {
            // x = 0
            positions[0] = glm::vec3{0, 0, 0};
            positions[1] = glm::vec3{0, 0, size};
            positions[2] = glm::vec3{0, size, 0};
            positions[3] = glm::vec3{0, size, size};

            normal = glm::vec3{-1, 0, 0};
            tangent = glm::vec3{0, 0, 1};

        } else if(which_face == face_id::RIGHT) {
            // x = 0
            positions[0] = glm::vec3{size, 0, 0};
            positions[1] = glm::vec3{size, 0, size};
            positions[2] = glm::vec3{size, size, 0};
            positions[3] = glm::vec3{size, size, size};

            normal = glm::vec3{1, 0, 0};
            tangent = glm::vec3{0, 0, -1};

        } else if(which_face == face_id::BOTTOM) {
            // y = 0
            positions[0] = glm::vec3{0, 0, 0};
            positions[1] = glm::vec3{0, 0, size};
            positions[2] = glm::vec3{size, 0, 0};
            positions[3] = glm::vec3{size, 0, size};

            normal = glm::vec3{0, -1, 0};
            tangent = glm::vec3{-1, 0, 0};

        } else if(which_face == face_id::TOP) {
            // y = 0
            positions[0] = glm::vec3{0, size, 0};
            positions[1] = glm::vec3{0, size, size};
            positions[2] = glm::vec3{size, size, 0};
            positions[3] = glm::vec3{size, size, size};

            normal = glm::vec3{0, 1, 0};
            tangent = glm::vec3{1, 0, 0};

        } else if(which_face == face_id::BACK) {
            // z = 0
            positions[0] = glm::vec3{0, 0, 0};
            positions[1] = glm::vec3{0, size, 0};
            positions[2] = glm::vec3{size, 0, 0};
            positions[3] = glm::vec3{size, size, 0};

            normal = glm::vec3{0, 0, -1};
            tangent = glm::vec3{-1, 0, 0};

        } else if(which_face == face_id::FRONT) {
            // z = 0
            positions[0] = glm::vec3{0, 0, size};
            positions[1] = glm::vec3{0, size, size};
            positions[2] = glm::vec3{size, 0, size};
            positions[3] = glm::vec3{size, size, size};

            normal = glm::vec3{0, 0, 1};
            tangent = glm::vec3{1, 0, 0};
        }

        auto face = block_face{};
        for(int i = 0; i < 4; i++) {
            face.vertices[i].position = positions[i];
            face.vertices[i].normal = normal;
            face.vertices[i].tangent = tangent;
        }

        return face;
    }

	bool is_cube(const glm::ivec3 pos, const mc_chunk& chunk) {
		return true;
	}

	float get_ao_in_direction(const glm::vec3 position, const face_id face_to_check, const mc_chunk& chunk) {
		return 0;
	}

    el::base::Writer &operator<<(el::base::Writer &out, const block_vertex& vert) {
        out << "block_vertex { position=" << vert.position << ", uv=" << vert.uv << ", lightmap_uv="
            << vert.lightmap_uv << ", normal=" << vert.normal << ", tangent=" << vert.tangent << "}";

        return out;
    }
}
