#include "glfw_window.hpp"

#include "nova_renderer/nova_renderer.hpp"
#include "../util/logger.hpp"

void glfw_error_callback(const int error, const char* desc) {
    NOVA_LOG(ERROR) << "GLFW error(" << error << ") " << desc;
}

namespace nova::renderer {
    GlfwWindow::GlfwWindow(const NovaSettings& options) {
        if(!glfwInit()) {
            NOVA_LOG(FATAL) << "Failed to init GLFW";
            return;
        }

        glfwSetErrorCallback(glfw_error_callback);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        if(options.debug.enabled) {
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        }

        window = glfwCreateWindow(static_cast<int>(options.window.width), static_cast<int>(options.window.height),
                options.window.title, nullptr, nullptr);
        if(!window) {
            NOVA_LOG(FATAL) << "Failed to create glfw window";
            return;
        }

        glfwMakeContextCurrent(window);
    }

    GlfwWindow::~GlfwWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void GlfwWindow::on_frame_end() {
        glfwSwapBuffers(window);
    }

    bool GlfwWindow::should_close() const {
        return glfwWindowShouldClose(window);
    }

    glm::uvec2 GlfwWindow::get_window_size() const {
        int width;
        int height;
        glfwGetFramebufferSize(window, &width, &height);

        return {width, height};
    }

    void* GlfwWindow::get_gl_proc_address(const char *proc_name) {
        return reinterpret_cast<void *>(glfwGetProcAddress(proc_name));
    }
}
