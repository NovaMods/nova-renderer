/*!
 * \brief
 *
 * \author ddubois 
 * \date 17-Jan-17.
 */

#ifndef RENDERER_TEST_UTILS_H
#define RENDERER_TEST_UTILS_H


#include <gtest/gtest.h>
#include <memory>
#include "../mc_interface/mc_gui_objects.h"
#include "../mc_interface/mc_objects.h"

namespace nova {
    namespace test {
        const std::string TEST_RESOURCES_LOCATION = "../src/test/resources/";

        mc_gui_screen get_gui_screen_one_button();

        /*!
         * \brief Loads a single chunk from the TEST_RESOURCES_LOCATION
         *
         * \param chunk_file The filename of the chunk to load
         * \return The chunk
         */
        std::shared_ptr<mc_chunk> load_test_chunk(std::string chunk_file);

        /*!
         * \brief A base class for test cases that need to run with Nova running
         */
        class nova_test : public ::testing::Test {
        public:
            virtual void SetUp();
            virtual void TearDown();
        };
    }
}

#endif //RENDERER_TEST_UTILS_H
