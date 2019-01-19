//
// Created by jannis on 21.08.18.
//

#include "../../src/general_test_setup.hpp"

#undef TEST
#include <gtest/gtest.h>

TEST(nova_filesystem, zip_reading) {
    TEST_SETUP_LOGGER();

    auto file_test = nova::zip_folder_accessor(fs::path{"shaderpacks/DefaultShaderpack.zip"});
    std::vector<fs::path> files = file_test.get_all_items_in_folder({"materials"});
    for (const auto &file : files) {
        NOVA_LOG(INFO) << file.string();
    }
}