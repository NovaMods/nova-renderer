/*!
 * \brief
 *
 * \author ddubois 
 * \date 17-Jan-17.
 */

#ifndef RENDERER_TEST_UTILS_H
#define RENDERER_TEST_UTILS_H


#include <gtest/gtest.h>
#include "../mc_interface/mc_gui_objects.h"

namespace nova {
    namespace test {
        mc_gui_screen get_gui_screen_one_button();

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
