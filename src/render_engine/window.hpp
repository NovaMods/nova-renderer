/*!
 * \author ddubois
 * \date 01-Sep-18.
 */

#ifndef NOVA_RENDERER_WINDOW_HPP
#define NOVA_RENDERER_WINDOW_HPP

#include "../util/utils.hpp"

namespace nova {
    NOVA_EXCEPTION(window_creation_exception);

    /*!
     * \brief A platform-independent window interface
     */
    class iwindow {
    public:

        /*!
         * \brief Handles what should happen when the frame is done. This includes telling the operating system that
         * we're still alive
         */
        virtual void on_frame_end() = 0;

        /*!
         * \brief Returns true if the window should close
         *
         * While a fully native program can handle program shutdown entirely on its own, Nova needs a way for the game
         * it's running in to know if the user has requested window closing. This method is that way
         */
        virtual bool should_close() const = 0;

        /*!
         * \brief Gets the current window size
         *
         * \return The current size of the window
         */
        virtual glm::uvec2 get_window_size() const = 0;
    };
}  // namespace nova

#endif  // NOVA_RENDERER_WINDOW_HPP
