#pragma once

#include <string>
#include <cstdint>

namespace nova::filesystem {
    inline std::string get_file_name(const std::string& path) {
        const auto& path_parts = path.split('/');
        return path_parts.last();
    }
} // namespace nova::filesystem
