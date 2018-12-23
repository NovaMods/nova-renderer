//
// Created by ddubois on 12/23/18.
//

#ifndef NOVA_RENDERER_GLFW_WINDOW_H
#define NOVA_RENDERER_GLFW_WINDOW_H

#include "window.hpp"

namespace nova {
    /*!
     * \brief A window class that uses GLFW
     */
    class glfw_window : public iwindow {
    public:
        glfw_window();

        void on_frame_end() override;

        bool should_close() const override;

        glm::uvec2 get_vindow_size() const override;

    };
}

#endif //NOVA_RENDERER_GLFW_WINDOW_H
