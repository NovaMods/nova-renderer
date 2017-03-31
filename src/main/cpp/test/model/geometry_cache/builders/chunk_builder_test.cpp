/*!
 * \brief Tests the functions in geometry_cache/builders/chunk_builder.h
 *
 * \author ddubois 
 * \date 26-Mar-17.
 */

#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include "../../../../geometry_cache/builders/chunk_builder.h"

namespace nova {
    namespace test {
         TEST(chunk_builder_test, get_if_block_at_pos_is_opaque_x_small_test) {
            glm::ivec3 small_x = glm::ivec3(-1, 8, 8);

            auto val = get_if_block_at_pos_is_opaque(small_x, mc_chunk());

            ASSERT_EQ(val, true);
        }

        TEST(chunk_builder_test, get_if_block_at_pos_is_opaque_x_large_test) {
            glm::ivec3 large_x = glm::ivec3(CHUNK_WIDTH * 2, 8, 8);

            auto val = get_if_block_at_pos_is_opaque(large_x, mc_chunk());

            ASSERT_EQ(val, true);
        }

        TEST(chunk_builder_test, get_if_block_at_pos_is_opaque_y_small_test) {
            glm::ivec3 small_y = glm::ivec3(8, -1, 8);

            auto val = get_if_block_at_pos_is_opaque(small_y, mc_chunk());

            ASSERT_EQ(val, true);
        }

        TEST(chunk_builder_test, get_if_block_at_pos_is_opaque_y_large_test) {
            glm::ivec3 large_y = glm::ivec3(8, CHUNK_HEIGHT * 2, 8);

            auto val = get_if_block_at_pos_is_opaque(large_y, mc_chunk());

            ASSERT_EQ(val, true);
        }

        TEST(chunk_builder_test, get_if_block_at_pos_is_opaque_z_small_test) {
            glm::ivec3 small_z = glm::ivec3(8, 8, -1);

            auto val = get_if_block_at_pos_is_opaque(small_z, mc_chunk());

            ASSERT_EQ(val, true);
        }

        TEST(chunk_builder_test, get_if_block_at_pos_is_opaque_z_large_test) {
            glm::ivec3 large_z = glm::ivec3(8, 8, CHUNK_DEPTH * 2);

            auto val = get_if_block_at_pos_is_opaque(large_z, mc_chunk());

            ASSERT_EQ(val, true);
        }
    }
}
