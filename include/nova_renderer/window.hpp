#pragma once

#include <functional>

#include <glm/vec2.hpp>

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
    class Window {
    public:
        explicit Window(const NovaSettings& options);
        ~Window();
        Window(Window&& other) noexcept = delete;
        Window& operator=(Window&& other) noexcept = delete;

        Window(const Window& other) = delete;
        Window& operator=(const Window& other) = delete;

        /*!
         * \brief Registers a new key input callback
         *
         * This callback will key called for every key press event that this window receives
         *
         * \param key_callback Callback for when a key is received. Intentionally a std::function so I can easily add
         * it to a vector
         */
        void register_key_callback(std::function<void(uint32_t)>&& key_callback);

        /*!
         * \brief Sends the provided key to all registered key callbacks
         */
        void process_key(int key);

        void poll_input() const;

        [[nodiscard]] bool should_close() const;

        [[nodiscard]] glm::uvec2 get_window_size() const;

#if NOVA_WINDOWS
        [[nodiscard]] HWND get_window_handle() const;

#elif NOVA_LINUX
        [[nodiscard]] Window get_window_handle() const;

        [[nodiscard]] Display* get_display() const;

#endif

#if NOVA_OPENGL_RHI
        void swap_backbuffer() const;

        static void* get_gl_proc_address(const char* proc_name);
#endif

    private:
        GLFWwindow* window = nullptr;

        std::vector<std::function<void(uint32_t)>> key_callbacks;
    };
} // namespace nova::renderer
