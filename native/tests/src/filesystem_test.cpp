//
// Created by jannis on 21.08.18.
//

#include <gtest/gtest.h>
#include "../../src/loading/regular_folder_accessor.hpp"

TEST(nova_filesystem, invalid_folder) {
    nova::regular_folder_accessor accessor("");
}