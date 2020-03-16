#include "../../src/general_test_setup.hpp"

#undef TEST
#include <gtest/gtest.h>

RX_LOG("FilesystemTest", logger);

TEST(NovaFilesystem, ZipReading) {
    auto file_test = nova::filesystem::ZipFolderAccessor(rx::string{CMAKE_DEFINED_RESOURCES_PREFIX "shaderpacks/DefaultShaderpack.zip"});
    const auto files = file_test.get_all_items_in_folder({"materials"});
    files.each_fwd([](const rx::string& file) { logger->info("%s", file); });
}
