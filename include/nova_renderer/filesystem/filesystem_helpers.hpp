#pragma once

#include <rx/core/string.h>
#include <stdint.h>

namespace nova::filesystem {
    /*!
     * \brief Combines all provided strings into a single path, inserting "/" in between them
     */
    template <typename... Args>
    rx::string combine_path(const Args&... parts);

    /*!
     * \brief Combines all provided strings into a single path, inserting "/" between them
     *
     * The memory for the output string will be allocated from the provided allocator
     *
     * All the Args types _should_ be trivially convertible to `rs::string`. I don't know how to check for that and I don't want to learn
     */
    template <typename... Args>
    rx::string combine_path(rx::memory::allocator* allocator, const Args&... parts);

    void copy_path_parts_to_buffer(rx_size offset, char* buffer, const rx::string& path_part);

    template <typename... Args>
    void copy_path_parts_to_buffer(rx_size offset, char* buffer, const rx::string& path_part, const Args&... parts);

    inline rx::string get_file_name(const rx::string& path) {
        const auto& path_parts = path.split('/');
        return path_parts.last();
    }

    template <typename... Args>
    rx::string combine_path(const Args&... parts) {
        return combine_path(&rx::memory::g_system_allocator, parts...);
    }

    template <typename... Args>
    rx::string combine_path(rx::memory::allocator* allocator, const Args&... parts) {
        const auto final_string_size = va_count(parts);

        auto* buffer = reinterpret_cast<char*>(allocator->allocate(final_string_size));

        copy_path_parts_to_buffer(0, buffer, parts);

        return {allocator, buffer, final_string_size};
    }

    template <typename... Args>
    void copy_path_parts_to_buffer(const rx_size offset, char* buffer, const rx::string& path_part, const Args&... parts) {
        const auto string_length = path_part.size();
        memcpy(buffer + offset, path_part.data(), string_length);
        buffer[string_length] = '/';

        copy_path_parts_to_buffer(offset + string_length + 1, buffer, parts);
    }
} // namespace nova::filesystem
