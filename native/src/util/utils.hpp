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

#include "macros.hpp"

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

    template<typename Out>
    void split(const std::string &s, char delim, Out result) {
        std::stringstream ss;
        ss.str(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            *(result++) = item;
        }
    }

    std::vector<std::string> split(const std::string &s, char delim);

    /*!
     * \brief Simple exception to represent that a resource can not be found
     */
    NOVA_EXCEPTION(nova, resource_not_found)

    std::string print_color(unsigned int color);

    std::string print_array(int data[], int num_elements);
}

#endif //RENDERER_UTILS_H