/*!
 * \author cwfitzgerald
 * \date 27-Jan-19.
 */

#ifndef NOVA_RENDERER_FILESYSTEM_UTIL_HPP
#define NOVA_RENDERER_FILESYSTEM_UTIL_HPP

#if NOVA_STD_FILESYSTEM
#include <filesystem>
namespace fs = std::filesystem;
#elif NOVA_STD_FILESYSTEM_EXPERIMENTAL
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#endif
