//
// Created by jannis on 01.09.18.
//

#ifndef NOVA_RENDERER_X_11_WINDOW_HPP
#define NOVA_RENDERER_X_11_WINDOW_HPP

#include "../../util/macros.hpp"
#include <X11/Xlib.h>
#include <stdint.h>

NOVA_EXCEPTION(nova, x_window_creation_exception)

namespace nova {
    class x11_window {
    private:
        Window window;
        Display *display;

    public:
        explicit x11_window(uint32_t width, uint32_t height);
        ~x11_window();

        Window &get_x11_window();
        Display *get_display();

        void enter_loop();
    };
}


#endif //NOVA_RENDERER_X_11_WINDOW_HPP
