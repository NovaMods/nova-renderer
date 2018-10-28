/*!
 * \brief Contains a bunch of utility functions which may or may not be actually used anywhere
 *
 * \author David
 * \date 18-May-16.
 */

#ifndef RENDERER_UTILS_H
#define RENDERER_UTILS_H

#include <vector>
#include <string>
#include <algorithm>
#include <exception>
#include <optional>

#include <fstream>

#include <glm/glm.hpp>
#include <sstream>

#include <filesystem>

#if _WIN32
    #if _MSC_VER <= 1915
        namespace fs = std::experimental::filesystem;
    #else
        namespace fs = std::filesystem;
    #endif
#else
    namespace fs = std::filesystem;
#endif

namespace nova {
    /*!
     * \brief Calls the function once for every element in the provided container
     *
     * \param container The container to perform an action for each element in
     * \param thingToDo The action to perform for each element in the collection
     */
    template <typename Cont, typename Func>
    void foreach(Cont container, Func thingToDo) {
        std::for_each(std::cbegin(container), std::cend(container), thingToDo);
    }

    std::vector<std::string> split(const std::string &s, char delim);

    std::string join(const std::vector<std::string> &strings, const std::string &joiner);

    std::string print_color(unsigned int color);

    std::string print_array(int data[], int num_elements);

    bool ends_with(const std::string &string, const std::string &ending);

    void write_to_file(const std::string& data, const fs::path& filepath);

    void write_to_file(const std::vector<uint32_t>& data, const fs::path& filepath);

    class nova_exception : public std::exception {
    private:
        std::string msg;

    public:
        explicit nova_exception(std::string msg);
        const char *what() const noexcept override;
    };

#define NOVA_EXCEPTION(name)                                                                                                                                                                           \
    class name : public ::nova::nova_exception {                                                                                                                                                       \
    public:                                                                                                                                                                                            \
        explicit name(std::string msg) : ::nova::nova_exception(std::move(msg)){};                                                                                                                     \
    }
}  // namespace nova

#endif  // RENDERER_UTILS_H