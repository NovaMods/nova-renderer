//
// Created by jannis on 01.09.18.
//

#include "x11_window.hpp"

namespace nova {
    x11_window::x11_window(uint32_t width, uint32_t height) {
        Display *display = XOpenDisplay(nullptr);
        if(!display) {
            throw x_window_creation_exception("Failed to open XDisplay");
        }

        int screen = DefaultScreen(display);
        window = XCreateSimpleWindow(display, RootWindow(display, screen), )
    }

    Window &x11_window::get_x11_window() {
        return window;
    }
}