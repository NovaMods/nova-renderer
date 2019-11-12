#pragma once

#ifdef NOVA_LINUX

#include <cstdint>
#include "nova_renderer/window.hpp"
#include "nova_renderer/nova_settings.hpp"

#include <X11/Xlib.h>

namespace nova::renderer {
    class X11Window final : public Window {
    private:
        ::Window window; // X11 window type!
        Display* display;
        bool should_window_close = false;
        Atom wm_protocols;
        Atom wm_delete_window;

    public:
        explicit X11Window(const NovaSettings::WindowOptions &options);

        X11Window(X11Window&& other) noexcept = delete;
        X11Window& operator=(X11Window&& other) noexcept = delete;

        X11Window(const X11Window& other) = delete;
        X11Window& operator=(const X11Window& other) = delete;

        virtual ~X11Window();

        ::Window& get_x11_window();
        Display* get_display();

        void on_frame_end() override;
        [[nodiscard]] bool should_close() const override;

        [[nodiscard]] glm::uvec2 get_window_size() const override;
    };
} // namespace nova::renderer

#endif

// X11 macros that are bad
#ifdef Always
#undef Always
#endif

#ifdef None
#undef None
#endif

#ifdef Bool
#undef Bool
#endif