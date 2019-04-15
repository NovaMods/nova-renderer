/*!
 * \brief Contains a bunch of utility functions which may or may not be actually used anywhere
 *
 * \author David
 * \date 18-May-16.
 */

#ifndef RENDERER_UTILS_H
#define RENDERER_UTILS_H

#include <algorithm>
#include <exception>
#include <optional>
#include <string>
#include <vector>

#include <fstream>

#include "filesystem.hpp"

#if defined(NOVA_EXPORT)
#define NOVA_API __declspec(dllexport)
#else
#define NOVA_API __declspec(dllimport)
#endif

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

    class NovaException : public std::exception {
    private:
        std::string msg;

        std::string generate_msg(const std::string& msg, const std::optional<std::exception>& exception);

    public:
        NovaException();
        explicit NovaException(const std::string& msg);

        explicit NovaException(const std::exception& cause);
        NovaException(const std::string& msg, const std::exception& cause);
        [[nodiscard]] const char* what() const noexcept override final;
    };

#define FORMAT(s, ...) fmt::format(fmt(s), __VA_ARGS__)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NOVA_EXCEPTION(name)                                                                                                               \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses)*/                                                                                        \
    class name : public ::nova::renderer::NovaException {                                                                                  \
    public:                                                                                                                                \
        name(){};                                                                                                                          \
        explicit name(const std::string& msg) : ::nova::renderer::NovaException(msg){};                                                    \
                                                                                                                                           \
        explicit name(const std::exception& cause) : ::nova::renderer::NovaException(cause){};                                             \
        name(const std::string& msg, const std::exception& cause) : ::nova::renderer::NovaException(msg, cause){};                         \
    }

    NOVA_EXCEPTION(out_of_gpu_memory);
} // namespace nova::renderer

#endif // RENDERER_UTILS_H