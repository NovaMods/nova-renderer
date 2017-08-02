/*!
 * \brief Tests the functions in utils/io.h
 *
 * \author gold1 
 * \date 10-Jun-17.
 */

#include <gtest/gtest.h>
#include "../../mc_interface/mc_objects.h"
#include "../../utils/io.h"
#include "../test_utils.h"

namespace nova {
    namespace test {
        TEST(io_test, save_chunk_all_air) {
            auto* chunk = new mc_basic_render_object{};
            chunk->chunk_id = 42;
            for(int i = 0; i < CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH; i++) {
                chunk->blocks[i] = mc_block{0, 0, false, "tile:air"};
            }
            auto filename = std::string{"save_chunk_one_block_test.json"};
            save_chunk(*chunk, filename);
        }
    }
}
