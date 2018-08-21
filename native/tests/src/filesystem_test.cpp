//
// Created by jannis on 21.08.18.
//

#include <gtest/gtest.h>
#include "../../src/fs_access/regular_folder_accessor.hpp"

TEST(nova_filesystem, invalid_folder) {
    nova::regular_folder_accessor accessor("");
}