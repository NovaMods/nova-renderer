#pragma once

// TODO: Figure out how to get rid of this include
#include <functional>

#include <glm/vec2.hpp>
#include <rx/core/vector.h>

#include "nova_renderer/nova_settings.hpp"
#include "nova_renderer/util/platform.hpp"
#include "nova_renderer/window.hpp"
#if NOVA_WINDOWS
#include "nova_renderer/util/windows.hpp"

#elif NOVA_LINUX
#include "nova_renderer/util/x11_but_good.hpp"
#endif

// ReSharper disable once CppInconsistentNaming
struct GLFWwindow;

namespace nova::renderer {
    /**
     * \brief GLFW wrapper that does a few nice things that Nova likes
     */
    class NovaWindow {
    public:
        explicit NovaWindow(const NovaSettings& options);
        ~NovaWindow();
        NovaWindow(NovaWindow&& other) noexcept = delete;
        NovaWindow& operator=(NovaWindow&& other) noexcept = delete;

        NovaWindow(const NovaWindow& other) = delete;
        NovaWindow& operator=(const NovaWindow& other) = delete;

        /*!
         * \brief Registers a new key input callback
         *
         * This callback will key called for every key press event that this window receives
         *
         * \param key_callback Callback for when a key is received. Intentionally a std::function so I can easily add
         * it to a vector. First parameter to this function is the key code, second is whether the key was pressed
         * this frame, third is if control is down, fourth is if shift is down
         */
        void register_key_callback(std::function<void(uint32_t, bool, bool, bool)> key_callback);

        /*!
         * \brief Registers a new mouse position callback
         *
         * Mouse position callback gets called when the mouse position changed
         *
         * \param mouse_callback Callback for when mouse input is received. The first parameter is the mouse's X
         * position, the second if the Y position
         */
        void register_mouse_callback(std::function<void(double, double)> mouse_callback);

        /*!
         * \brief Registers a new mouse button callback
         *
         * This callback gets invoked whenever the user presses a mouse button
         *
         * \param mouse_callback Callback for when a mouse button is pressed. First parameter is the mouse button,
         * second parameter is if it was pressed
         */
        void register_mouse_button_callback(std::function<void(uint32_t, bool)> mouse_callback);

        void poll_input() const;

        [[nodiscard]] bool should_close() const;

        /*!
         * \brief Gets the size of the framebuffer that this window displays
         */
        [[nodiscard]] glm::uvec2 get_framebuffer_size() const;

        /*!
         * \brief Gets the size of the window itself
         */
        [[nodiscard]] glm::uvec2 get_window_size() const;

        /*!
         * \brief Get the ratio of the size of the framebuffer to the size of the window
         */
        [[nodiscard]] glm::vec2 get_framebuffer_to_window_ratio() const;

#if NOVA_WINDOWS
        [[nodiscard]] HWND get_window_handle() const;

#elif NOVA_LINUX
        [[nodiscard]] Window get_window_handle() const;

        [[nodiscard]] Display* get_display() const;

#endif

    private:
        GLFWwindow* window = nullptr;

        std::vector<std::function<void(uint32_t, bool, bool, bool)>> key_callbacks;

        std::vector<std::function<void(double, double)>> mouse_callbacks;

        std::vector<std::function<void(uint32_t, bool)>> mouse_button_callbacks;

        static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

        static void glfw_mouse_callback(GLFWwindow* window, double x_position, double y_position);

        static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

        void broadcast_key_event(int key, bool is_press, bool is_control_down, bool is_shift_down);

        void broadcast_mouse_position(double x_position, double y_position);

        void broadcast_mouse_button(int button, bool is_pressed);
    };
} // namespace nova::renderer
