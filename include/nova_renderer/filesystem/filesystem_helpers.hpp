#pragma once

#include <rx/core/string.h>
#include <stdint.h>

namespace nova::filesystem {
    inline rx::string get_file_name(const rx::string& path) {
        const auto& path_parts = path.split('/');
        return path_parts.last();
    }
} // namespace nova::filesystem
