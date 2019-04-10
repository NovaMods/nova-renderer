#include "nova_renderer/util/platform.hpp"

#ifdef NOVA_LINUX

#include "x11_window.hpp"

namespace nova::renderer {
    x11_window::x11_window(uint32_t width, uint32_t height, const std::string& title) {
        display = XOpenDisplay(nullptr);
        if(display == nullptr) {
            throw window_creation_error("Failed to open XDisplay");
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        int screen = DefaultScreen(display);

        window = XCreateSimpleWindow(display,
                                     RootWindow(display, screen), // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                                     50,
                                     50,
                                     width,
                                     height,
                                     1,
                                     BlackPixel(display, screen),  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                                     WhitePixel(display, screen)); // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)

        XStoreName(display, window, title.c_str());

        wm_protocols = XInternAtom(display, "WM_PROTOCOLS", 0);
        wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
        XSetWMProtocols(display, window, &wm_delete_window, 1);

        XSelectInput(display, window, ExposureMask | ButtonPressMask | KeyPressMask);
        XMapWindow(display, window);
    }

    x11_window::~x11_window() {
        XUnmapWindow(display, window);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
    }

    Window& x11_window::get_x11_window() { return window; }

    Display* x11_window::get_display() { return display; }

    void x11_window::on_frame_end() {
        XEvent event;
        while(XPending(display) != 0) {
            XNextEvent(display, &event);
            switch(event.type) {
                case ClientMessage: {
                    if(event.xclient.message_type == wm_protocols && event.xclient.data.l[0] == static_cast<long>(wm_delete_window)) {
                        should_window_close = true;
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    bool x11_window::should_close() const { return should_window_close; }

    glm::uvec2 x11_window::get_window_size() const {
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
} // namespace nova::renderer

#endif
