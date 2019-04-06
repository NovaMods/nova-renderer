#pragma once

#include <glm/glm.hpp>

#include "util/utils.hpp"

namespace nova::renderer {
    NOVA_EXCEPTION(window_creation_error);

    /*!
     * \brief A platform-independent window interface
     */
    class window_t {
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
        [[nodiscard]] virtual bool should_close() const = 0;

        /*!
         * \brief Gets the current window size
         *
         * \return The current size of the window
         */
        [[nodiscard]] virtual glm::uvec2 get_window_size() const = 0;
    };
} // namespace nova::renderer
