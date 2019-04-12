#pragma once

#if NOVA_STD_FILESYSTEM
#include <filesystem>
namespace fs = std::filesystem;
#elif NOVA_STD_FILESYSTEM_EXPERIMENTAL
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
