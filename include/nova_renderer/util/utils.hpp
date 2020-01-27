#pragma once

// MUST be before <algorithm> to keep gcc happy
#include <algorithm>
#include <memory_resource>
#include <string>
#include <vector>

#include <rx/core/optional.h>

namespace rx {
    struct string;
}

namespace nova::renderer {
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

    std::pmr::vector<std::string> split(const std::string& s, char delim);

    std::string join(const std::pmr::vector<std::string>& strings, const std::string& joiner);

    std::string print_color(unsigned int color);

    std::string print_array(int* data, int size);

    bool ends_with(const std::string& string, const std::string& ending);

    template <typename ValueType, typename... Args>
    inline rx::optional<ValueType> make_optional(Args... args);

    template <typename ValueType, typename... Args>
    inline rx::optional<ValueType> make_optional(Args... args) {
        return rx::optional<ValueType>(ValueType{rx::utility::forward<Args>(args)...});
    }

#define FORMAT(s, ...) fmt::format(fmt(s), __VA_ARGS__)

} // namespace nova::renderer
