#include <gtest/gtest.h>

#include "../src/general_test_setup.hpp"

int main(int argc, char** argv) {
    TEST_CONFIGURE_RUNTIME();
    init_rex();

    testing::InitGoogleTest(&argc, argv);
    const auto res = RUN_ALL_TESTS();

    rex_fini();

    return res;
}