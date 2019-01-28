/*!
 * \author cwfitzgerald
 * \date 27-Jan-19.
 */

#ifndef NOVA_RENDERER_FILESYSTEM_UTIL_HPP
#define NOVA_RENDERER_FILESYSTEM_UTIL_HPP

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#endif
