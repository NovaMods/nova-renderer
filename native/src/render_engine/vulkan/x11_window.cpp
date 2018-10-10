//
// Created by jannis on 01.09.18.
//

#include "x11_window.hpp"

namespace nova {
    x11_window::x11_window(uint32_t width, uint32_t height) {
        display = XOpenDisplay(nullptr);
        if(!display) {
            throw window_creation_exception("Failed to open XDisplay");
        }

        int screen = DefaultScreen(display);
        window = XCreateSimpleWindow(display, RootWindow(display, screen), 50, 50, width, height, 1, BlackPixel(display, screen), WhitePixel(display, screen));

        wm_protocols = XInternAtom(display, "WM_PROTOCOLS", false);
        wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", false);
        XSetWMProtocols(display, window, &wm_delete_window, 1);

        XSelectInput(display, window, ExposureMask | ButtonPressMask | KeyPressMask);
        XMapWindow(display, window);
    }

    x11_window::~x11_window() {
        XUnmapWindow(display, window);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
    }

    Window &x11_window::get_x11_window() {
        return window;
    }

    Display *x11_window::get_display() {
        return display;
    }

    void x11_window::on_frame_end() {
        XEvent event;
        while(XPending(display)) {
            XNextEvent(display, &event);
            switch (event.type) {
                case ClientMessage: {
                    if (event.xclient.message_type == wm_protocols && event.xclient.data.l[0] == wm_delete_window) {
                        should_window_close = true;
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    bool x11_window::should_close() const {
        return should_window_close;
    }

    iwindow::window_size x11_window::get_window_size() const {
        Window root_window;
        int x_pos;
        int y_pos;
        uint32_t width;
        uint32_t height;
        uint32_t border_width;
        uint32_t depth;

        XGetGeometry(display, window, &root_window, &x_pos, &y_pos, &width, &height, &border_width, &depth);

        return {width, height};
    }
}