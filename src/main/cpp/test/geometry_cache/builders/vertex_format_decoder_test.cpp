/*!
 * \brief Tests the functions in vertex_format_decoder.h
 *
 * \author gold1 
 * \date 02-Jul-17.
 */

#include <gtest/gtest.h>
#include "../../../geometry_cache/builders/vertex_format_decoder.h"

namespace nova {
    namespace test {
        TEST(vertex_format_decoder, decode_block_vertices_real_data) {
            int * data = new int[28] {
                    0, 0, 1065353216, -8421505, 1027607101, 1050673807, 0,
                    0, 0, 0, -8421505, 1027607101, 1051721073, 0,
                    1065353216, 0, 0, -8421505, 1031796163, 1051721073, 0,
                    1065353216, 0, 1065353216, -8421505, 1031796163, 1050673807, 0
            };

            auto vertices = decode_block_vertices(data, 28);

            EXPECT_EQ(vertices.size(), 4);
        }
    }
}

