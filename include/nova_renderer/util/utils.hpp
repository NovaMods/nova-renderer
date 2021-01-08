#pragma once

// MUST be before <algorithm> to keep gcc happy
#include <algorithm>
#include <memory_resource>
#include <string>
#include <vector>

#include  <optional>

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

#define FORMAT(s, ...) fmt::format(fmt(s), __VA_ARGS__)

#define PROFILE_VOID_EXPR(expr, category, event_name)                                                                                      \
    [&] {                                                                                                                                  \
        MTR_SCOPE(#category, #event_name);                                                                                                 \
        expr;                                                                                                                              \
    }()

#define PROFILE_RET_EXPR(expr, category, event_name)                                                                                       \
    [&] {                                                                                                                                  \
        MTR_SCOPE(#category, #event_name);                                                                                                 \
        return expr;                                                                                                                       \
    }()

} // namespace nova::renderer
