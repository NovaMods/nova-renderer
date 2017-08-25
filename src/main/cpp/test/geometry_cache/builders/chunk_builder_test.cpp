/*!
 * \brief Tests the functions in geometry_cache/builders/chunk_builder.h
 *
 * \author ddubois 
 * \date 26-Mar-17.
 */

#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include "../../../geometry_cache/builders/chunk_builder.h"
#include "../../test_utils.h"
#include <easylogging++.h>

namespace nova {
    namespace test {
        /*
        TEST(chunk_builder_test, x_small_test) {
            glm::ivec3 small_x = glm::ivec3(-1, 8, 8);

            auto* chunk = new mc_chunk_render_object();
            auto val = block_at_pos_is_opaque(small_x, *chunk);

            ASSERT_EQ(val, true);
        }

        TEST(chunk_builder_test, block_at_pos_is_opaque_x_large_test) {
            glm::ivec3 large_x = glm::ivec3(CHUNK_WIDTH * 2, 8, 8);

            auto* chunk = new mc_chunk_render_object();
            auto val = block_at_pos_is_opaque(large_x, *chunk);

            ASSERT_EQ(val, true);
        }

        TEST(chunk_builder_test, block_at_pos_is_opaque_y_small_test) {
            glm::ivec3 small_y = glm::ivec3(8, -1, 8);

            auto* chunk = new mc_chunk_render_object();
            auto val = block_at_pos_is_opaque(small_y, *chunk);

            ASSERT_EQ(val, true);
        }

        TEST(chunk_builder_test, block_at_pos_is_opaque_y_large_test) {
            glm::ivec3 large_y = glm::ivec3(8, CHUNK_HEIGHT * 2, 8);

            auto* chunk = new mc_chunk_render_object();
            auto val = block_at_pos_is_opaque(large_y, *chunk);

            ASSERT_EQ(val, true);
        }

        TEST(chunk_builder_test, block_at_pos_is_opaque_z_small_test) {
            glm::ivec3 small_z = glm::ivec3(8, 8, -1);

            auto* chunk = new mc_chunk_render_object();
            auto val = block_at_pos_is_opaque(small_z, *chunk);

            ASSERT_EQ(val, true);
        }

        TEST(chunk_builder_test, block_at_pos_is_opaque_z_large_test) {
            glm::ivec3 large_z = glm::ivec3(8, 8, CHUNK_DEPTH * 2);

            auto* chunk = new mc_chunk_render_object();
            auto val = block_at_pos_is_opaque(large_z, *chunk);

            ASSERT_EQ(val, true);
        }

        TEST(chunk_builder_test, block_at_pos_is_opaque_everything_large_test) {
            auto large_val = glm::ivec3{CHUNK_WIDTH - 1, CHUNK_HEIGHT - 1, CHUNK_DEPTH - 1};

            auto* chunk = new mc_chunk_render_object();
            auto val = block_at_pos_is_opaque(large_val, *chunk);

            ASSERT_EQ(val, true);
        }

		TEST(chunk_builder_test, pos_to_idx) {
			auto expected_idx = 1 + 1 * CHUNK_WIDTH + 1 * CHUNK_WIDTH * CHUNK_HEIGHT;

			auto pos = glm::ivec3{ 1, 1, 1 };

			auto actual_idx = nova::pos_to_idx(pos);

			ASSERT_EQ(expected_idx, actual_idx);
		}

        TEST(chunk_builder_test, make_geometry_for_block_one_block) {
            auto block_pos = glm::ivec3{5, 5, 5};

            auto* chunk = new mc_chunk_render_object{};
            auto block_idx = pos_to_idx(block_pos);

            chunk->blocks[block_idx] = mc_block{"stone", false, 5, 0, 0.0, true, true};

            auto faces = make_geometry_for_block(block_pos, *chunk);

            ASSERT_EQ(faces.size(), 6);
        }

        TEST(chunk_builder_test, make_mesh_for_blocks_one_block_vertex_sizes) {
            auto block_pos = glm::ivec3{5, 5, 5};
            auto block_poses = std::vector<glm::ivec3>{block_pos};

            auto* chunk = new mc_chunk_render_object{};
            auto block_idx = pos_to_idx(block_pos);

            chunk->blocks[block_idx] = mc_block{"stone", false, 5, 0, 0.0, true, true};

            auto mesh = make_mesh_for_blocks(block_poses, *chunk);

            ASSERT_EQ(mesh.vertex_format, format::POS_UV_LIGHTMAPUV_NORMAL_TANGENT);
            ASSERT_EQ(mesh.vertex_data.size(), 312);
            ASSERT_EQ(mesh.indices.size(), 36);
        }

        TEST(chunk_builder_test, make_mesh_for_blocks_one_block_air_around_vertex_sizes) {
            auto block_pos = glm::ivec3{5, 5, 5};
            auto block_poses = std::vector<glm::ivec3>{block_pos};

            auto* chunk = new mc_chunk_render_object{};
            auto block_idx = pos_to_idx(block_pos);

            for(int i = 0; i < CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH; i++) {
                chunk->blocks[i] = mc_block{"tile.air", false, 3, 0, 0, false, false};
            }

            chunk->blocks[block_idx] = mc_block{"stone", false, 5, 0, 0.0, true, true};

            auto mesh = make_mesh_for_blocks(block_poses, *chunk);

            ASSERT_EQ(mesh.vertex_format, format::POS_UV_LIGHTMAPUV_NORMAL_TANGENT);
            ASSERT_EQ(mesh.vertex_data.size(), 312);
            ASSERT_EQ(mesh.indices.size(), 36);
        }

        TEST(chunk_builder_test, make_mesh_for_blocks_one_block_vertex_positions) {
            auto block_pos = glm::ivec3{5, 5, 5};
            auto block_poses = std::vector<glm::ivec3>{block_pos};

            auto* chunk = new mc_chunk_render_object{};
            auto block_idx = pos_to_idx(block_pos);

            chunk->blocks[block_idx] = mc_block{"stone", false, 5, 0, 0.0, true, true};

            auto mesh = make_mesh_for_blocks(block_poses, *chunk);

            block_vertex* vert1 = (block_vertex *) &mesh.vertex_data[0];
            auto compare = glm::vec3{0, 1, 0} + glm::vec3{block_pos};
            ASSERT_EQ(vert1->position, compare);
        }

        TEST(chunk_builder_test, make_geometry_for_block_two_blocks) {
            auto block_pos = glm::ivec3{5, 5, 5};
            auto block_idx = pos_to_idx(block_pos);

            auto block2_idx = pos_to_idx(block_pos + glm::ivec3{1, 0, 0});

            auto* chunk = new mc_chunk_render_object{};

            chunk->blocks[block_idx] = mc_block{"stone", false, 5, 0, 0.0, true, true};
            chunk->blocks[block2_idx] = mc_block{"stone", false, 5, 0, 0.0, true, true};

            auto faces = make_geometry_for_block(block_pos, *chunk);

            ASSERT_EQ(faces.size(), 5);
        }

        TEST(chunk_builder_test, get_blocks_that_match_filter_gbuffers_terrain_filter) {
            auto chunk = load_test_chunk("0.chnk");

            auto filter = std::make_shared<and_geometry_filter>(std::make_shared<transparent_geometry_filter>(false),
                                                                std::make_shared<geometry_type_geometry_filter>(geometry_type::block));

            auto blocks_for_filter = get_blocks_that_match_filter(*chunk, filter);
            ASSERT_EQ(blocks_for_filter.size(), 16389);
        }
         */
    }
}
