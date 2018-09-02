/*!
 * \author ddubois 
 * \date 01-Sep-18.
 */

#ifndef NOVA_RENDERER_WINDOW_HPP
#define NOVA_RENDERER_WINDOW_HPP

namespace nova {
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
    };
}

#endif //NOVA_RENDERER_WINDOW_HPP
