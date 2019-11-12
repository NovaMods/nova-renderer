#pragma once

#include <GLFW/glfw3.h>
#include "nova_renderer/nova_settings.hpp"
#include "nova_renderer/window.hpp"

namespace nova::renderer {
    /**
     * \brief GLFW window for the OpenGL backend
     */
    class GlfwWindow final : public Window {
    private:
        GLFWwindow *window;

    public:
        explicit GlfwWindow(const NovaSettings& options);
        virtual ~GlfwWindow();

        GlfwWindow(GlfwWindow&& other) noexcept = delete;
        GlfwWindow& operator=(GlfwWindow&& other) noexcept = delete;

        GlfwWindow(const GlfwWindow& other) = delete;
        GlfwWindow& operator=(const GlfwWindow& other) = delete;

        void on_frame_end() override;
        [[nodiscard]] bool should_close() const override;
        [[nodiscard]] glm::uvec2 get_window_size() const override;

        static void* get_gl_proc_address(const char* proc_name);
    };
}