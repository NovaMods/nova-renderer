#include "nova_renderer/filesystem/filesystem_helpers.hpp"

#include <stdarg.h>
#include <string.h>

namespace nova::filesystem {
    void copy_path_parts_to_buffer(const rx_size offset, char* buffer, const rx::string& path_part) {
        memcpy(buffer + offset, path_part.data(), path_part.size());
    }
} // namespace nova::filesystem
