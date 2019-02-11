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

namespace nova {
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

    std::vector<std::string> split(const std::string &s, char delim);

    std::string join(const std::vector<std::string> &strings, const std::string &joiner);

    std::string print_color(unsigned int color);

    std::string print_array(int *data, int size);

    bool ends_with(const std::string &string, const std::string &ending);

    void write_to_file(const std::string &data, const fs::path &filepath);

    void write_to_file(const std::vector<uint32_t> &data, const fs::path &filepath);

    class nova_exception : public std::exception {
    private:
        std::string msg;

        std::string generate_msg(const std::string &msg, const std::optional<std::exception> &exception);

    public:
        nova_exception();
        explicit nova_exception(const std::string &msg);

        explicit nova_exception(const std::exception &cause);
        nova_exception(const std::string &msg, const std::exception &cause);
        [[nodiscard]] const char *what() const noexcept override;
    };

#define NOVA_EXCEPTION(name)                                                                                                               \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses)*/                                                                                        \
    class name : public ::nova::nova_exception {                                                                                           \
    public:                                                                                                                                \
        name(){};                                                                                                                          \
        explicit name(std::string msg) : ::nova::nova_exception(std::move(msg)){};                                                         \
                                                                                                                                           \
        explicit name(const std::exception &cause) : ::nova::nova_exception(cause){};                                                      \
        name(std::string msg, const std::exception &cause) : ::nova::nova_exception(std::move(msg), cause){};                              \
    }

    NOVA_EXCEPTION(out_of_gpu_memory);
} // namespace nova

#endif // RENDERER_UTILS_H