#include "../../src/general_test_setup.hpp"

#undef TEST
#include <gtest/gtest.h>

TEST(NovaFilesystem, ZipReading) {
    auto file_test = nova::renderer::ZipFolderAccessor(fs::path{CMAKE_DEFINED_RESOURCES_PREFIX "shaderpacks/DefaultShaderpack.zip"});
    std::pmr::vector<fs::path> files = file_test.get_all_items_in_folder({"materials"});
    for(const auto& file : files) {
        NOVA_LOG(INFO) << file.string();
    }
}