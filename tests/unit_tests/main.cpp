#include "../src/general_test_setup.hpp"

#include <gtest/gtest.h>

int main(int argc, char **argv) {
    TEST_CONFIGURE_RUNTIME();
    TEST_SETUP_LOGGER();

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}