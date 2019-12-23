#pragma once

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include "filesystem.hpp"

namespace nova::renderer {
    template <int Num>
    struct placeholder;

    /*!
     * \brief Calls the function once for every element in the provided container
     *
     * \param container The container to perform an action for each element in
     * \param thing_to_do The action to perform for each element in the collection
     */
    template <typename Cont, typename Func>
    void foreach(Cont container, Func thing_to_do) {
        std::for_each(std::cbegin(container), std::cend(container), thing_to_do);
    }

    std::vector<std::string> split(const std::string& s, char delim);

    std::string join(const std::vector<std::string>& strings, const std::string& joiner);

    std::string print_color(unsigned int color);

    std::string print_array(int* data, int size);

    bool ends_with(const std::string& string, const std::string& ending);

    void write_to_file(const std::string& data, const fs::path& filepath);

    void write_to_file(const std::vector<uint32_t>& data, const fs::path& filepath);

#define FORMAT(s, ...) fmt::format(fmt(s), __VA_ARGS__)
} // namespace nova::renderer
