#pragma once

#include <rx/core/string.h>

namespace nova {
    /*!
     * \brief Basic class for exceptions
     *
     * Exceptions indicate that Nova has encountered an error it cannot possibly recover from. This is things like the user not having a
     * suitable graphics card
     */
    class Exception {
    public:
        /*!
         * \brief Create an exception with a string message
         */
        Exception(rx::string msg);

        /*!
         * \brief Retrieve this exception's message
         */
        [[nodiscard]] virtual const rx::string& get_message() const;

    private:
        rx::string message;
    };
} // namespace nova
