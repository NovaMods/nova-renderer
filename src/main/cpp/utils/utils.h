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

/*!
 * \brief Initializes the logging system
 */
void initialize_logging();

namespace nova {
    /*!
     * \brief Calls the fucntion once for every element in the provided container
     *
     * \param container The container to perform an action for each element in
     * \param thingToDo The action to perform for each element in the collection
     */
    template <typename Cont, typename Func>
    void foreach(Cont container, Func thingToDo) {
        std::for_each(std::cbegin(container), std::cend(container), thingToDo);
    };

    /*!
     * \brief Simple exception to represent that a resouce can not be found
     */
    class resource_not_found : public std::exception {
    public:
        resource_not_found(const std::string& msg);
        virtual const char * what() const noexcept;
    private:
        std::string message;
    };

    template<typename T>
    class maybe {
    public:
        /*!
         * \brief Creates an empty maybe
         */
        maybe() : has_data(false) {}

        /*!
         * \brief Creates a Maybe which holds the given piece of data
         *
         * \param data The data for this Maybe to hold
         */
        template<typename T>
        maybe(T* data) : data(data), has_data(true) {}

        /*!
         * \brief Maps a function over this maybe
         *
         * \return A new maybe encapsulating the result of this function
         */
        template<typename T, typename U, typename Func>
        maybe<U> map(Func f) {
            if(!has_data) {
                return maybe();
            }

            return maybe(Func(data));
        };

        /*!
         * \brief Runs a function on the data in this maybe if it exists, doesn't otherwise
         * \param f
         */
        template<typename T, typename Func>
        void if_present(Func f) {
            if(has_data) {
                f(data);
            }
        };

        /*!
         * \brief Get the data in thie maybe, if it's present. If it isn't present, return the provided data
         */
        template<typename T>
        T* get_or_else(T* alternate) {
            return has_data ? data : alternate;
        }
    private:
        T* data;
        bool has_data;
    };
}

#endif //RENDERER_UTILS_H
