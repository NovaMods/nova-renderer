#include "nova_renderer/window.hpp"

#include <GLFW/glfw3.h>
#if NOVA_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif NOVA_LINUX
typedef int Bool; // Because X11 is stupid
#define GLFW_EXPOSE_NATIVE_X11
#endif
// We have to include this here so it exists before we #undef Bool, but ReSharper doesn't know the horrors of X11
// ReSharper disable once CppUnusedIncludeDirective
#include <GLFW/glfw3native.h>

#include "nova_renderer/nova_renderer.hpp"
#include "nova_renderer/util/logger.hpp"
#include "nova_renderer/util/platform.hpp"

void glfw_error_callback(const int error, const char* desc) { NOVA_LOG(ERROR) << "GLFW error(" << error << ") " << desc; }

namespace nova::renderer {
    void NovaWindow::glfw_key_callback(GLFWwindow* window, const int key, int /* scancode */, const int action, int /* mods */) {
        void* user_data = glfwGetWindowUserPointer(window);
        auto* my_window = static_cast<NovaWindow*>(user_data);

        const bool is_control_down = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                                     glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

        const bool is_shift_down = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                                   glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

        my_window->broadcast_key_event(key, action == GLFW_PRESS, is_control_down, is_shift_down);
    }

    void NovaWindow::glfw_mouse_callback(GLFWwindow* window, const double x_position, const double y_position) {
        void* user_data = glfwGetWindowUserPointer(window);
        auto* my_window = static_cast<NovaWindow*>(user_data);

        my_window->broadcast_mouse_position(x_position, y_position);
    }

    void NovaWindow::glfw_mouse_button_callback(GLFWwindow* window, const int button, const int action, int /* mods */) {
        void* user_data = glfwGetWindowUserPointer(window);
        auto* my_window = static_cast<NovaWindow*>(user_data);

        my_window->broadcast_mouse_button(button, action == GLFW_PRESS);
    }

    NovaWindow::NovaWindow(const NovaSettings& options) {
        if(!glfwInit()) {
            NOVA_LOG(FATAL) << "Failed to init GLFW";
            return;
        }

        glfwSetErrorCallback(glfw_error_callback);

        if(options.api == GraphicsApi::NvGl4) {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            if(options.debug.enabled) {
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
            }

        } else {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        window = glfwCreateWindow(static_cast<int>(options.window.width),
                                  static_cast<int>(options.window.height),
                                  options.window.title,
                                  nullptr,
                                  nullptr);
        if(!window) {
            NOVA_LOG(FATAL) << "Failed to create window";
            return;
        }

        if(options.api == GraphicsApi::NvGl4) {
            glfwMakeContextCurrent(window);
        }

        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, &NovaWindow::glfw_key_callback);
    }

    NovaWindow::~NovaWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void NovaWindow::register_key_callback(std::function<void(uint32_t, bool, bool, bool)> key_callback) {
        key_callbacks.emplace_back(std::move(key_callback));
    }

    void NovaWindow::register_mouse_callback(std::function<void(double, double)> mouse_callback) {
        mouse_callbacks.emplace_back(std::move(mouse_callback));
    }

    void NovaWindow::register_mouse_button_callback(std::function<void(uint32_t, bool)> mouse_callback) {
        mouse_button_callbacks.emplace_back(std::move(mouse_callback));
    }

    void NovaWindow::broadcast_key_event(const int key, const bool is_press, const bool is_control_down, const bool is_shift_down) {
        for(const auto& callback : key_callbacks) {
            callback(key, is_press, is_control_down, is_shift_down);
        }
    }

    void NovaWindow::broadcast_mouse_position(const double x_position, const double y_position) {
        for(const auto& callback : mouse_callbacks) {
            callback(x_position, y_position);
        }
    }

    void NovaWindow::broadcast_mouse_button(const int button, const bool is_pressed) {
        for(const auto& callback : mouse_button_callbacks) {
            callback(button, is_pressed);
        }
    }

    // This _can_ be static, but I don't want it to be
    // ReSharper disable once CppMemberFunctionMayBeStatic
    void NovaWindow::poll_input() const { glfwPollEvents(); }

    bool NovaWindow::should_close() const { return glfwWindowShouldClose(window); }

    glm::uvec2 NovaWindow::get_window_size() const {
        int width;
        int height;
        glfwGetFramebufferSize(window, &width, &height);

        return {width, height};
    }

#if NOVA_WINDOWS
    HWND NovaWindow::get_window_handle() const { return glfwGetWin32Window(window); }

#elif NOVA_LINUX
    Window NovaWindow::get_window_handle() const { return glfwGetX11Window(window); };

    Display* NovaWindow::get_display() const { return glfwGetX11Display(); };
#endif

#if NOVA_OPENGL_RHI
    void NovaWindow::swap_backbuffer() const { glfwSwapBuffers(window); }

    void* NovaWindow::get_gl_proc_address(const char* proc_name) { return reinterpret_cast<void*>(glfwGetProcAddress(proc_name)); }
#endif
} // namespace nova::renderer
