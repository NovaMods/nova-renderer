//
// Created by jannis on 01.09.18.
//

#include "x11_window.hpp"

namespace nova {
    x11_window::x11_window(uint32_t width, uint32_t height) {
        display = XOpenDisplay(nullptr);
        if(!display) {
            throw x_window_creation_exception("Failed to open XDisplay");
        }

        int screen = DefaultScreen(display);
        window = XCreateSimpleWindow(display, RootWindow(display, screen), 50, 50, width, height, 1, BlackPixel(display, screen), WhitePixel(display, screen));
        XSelectInput(display, window, ExposureMask | ButtonPressMask | KeyPressMask);
        XMapWindow(display, window);
    }

    x11_window::~x11_window() {
        XDestroyWindow(display, window);
        XCloseDisplay(display);
    }

    Window &x11_window::get_x11_window() {
        return window;
    }

    Display *x11_window::get_display() {
        return display;
    }

    void x11_window::enter_loop() {
        XEvent event;
        while(true) {
            XNextEvent(display, &event);
            if(event.type == KeyPress) {
                break;
            }
        }

        XUnmapWindow(display, window);
    }
}