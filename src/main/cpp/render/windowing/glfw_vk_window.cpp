//
// Created by David on 15-Apr-16.
//

#include <vulkan/vulkan.h>
#include "glfw_vk_window.h"
#include "../../utils/utils.h"

#include <easylogging++.h>
#include "../../input/InputHandler.h"
#include "../nova_renderer.h"
#include "../vulkan/render_context.h"

namespace nova {
    void error_callback(int error, const char *description) {
        LOG(ERROR) << "Error " << error << ": " << description;
    }

    void window_focus_callback(GLFWwindow *window, int focused) {
        glfw_vk_window::setActive((bool) focused);
    }

    bool glfw_vk_window::active = true;

    glfw_vk_window::glfw_vk_window() {
        initialize_logging();

        glfwSetErrorCallback(error_callback);

        if(glfwInit() == 0) {
            LOG(FATAL) << "Could not initialize GLFW";
        }
		init();
    }

    int glfw_vk_window::init() {

		nlohmann::json &config = nova_renderer::get_render_settings().get_options();

		float view_width = config["settings"]["viewWidth"];
		float view_height = config["settings"]["viewHeight"];

        if(!glfwVulkanSupported()) {
            LOG(FATAL) << "Your system does not support Vulkan. Nova will now exit";
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO: Deal with this later
        window = glfwCreateWindow((int)view_width, (int)view_height, "Minecraft Nova Renderer", NULL, NULL);
        if(window == nullptr) {
            LOG(FATAL) << "Could not initialize window :(";
        }
        LOG(INFO) << "GLFW window created";

#if __win32__
        renderdoc_manager = std::make_unique<RenderDocManager>(window, "C:\\Program Files\\RenderDoc\\renderdoc.dll", "capture");
        LOG(INFO) << "Hooked into RenderDoc";
#endif

        glfwSetKeyCallback(window, key_callback);
		glfwSetCharCallback(window, key_character_callback);
		glfwSetMouseButtonCallback(window, mouse_button_callback);
		glfwSetCursorPosCallback(window, mouse_position_callback);
        glfwSetScrollCallback(window, mouse_scroll_callback);
        glfwSetWindowFocusCallback(window, window_focus_callback);
		
		return 0;
    }

    glfw_vk_window::~glfw_vk_window() {
        destroy();
        
    }

    void glfw_vk_window::destroy() {
        glfwDestroyWindow(window);
        glfwTerminate();
        window = nullptr;
    }

    void glfw_vk_window::set_fullscreen(bool fullscreen) {
        GLFWmonitor* monitor = nullptr;
        int xPos = 0;
        int yPos = 0;
        int width;
        int height;

        if(fullscreen) {
            int oldXpos;
            int oldYpos;
            
            glfwGetWindowPos(window, &oldXpos, &oldYpos);
            
            windowed_window_parameters.xPos = oldXpos;
            windowed_window_parameters.yPos = oldYpos;
            windowed_window_parameters.width = window_dimensions.x;
            windowed_window_parameters.height = window_dimensions.y;

            monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            width = mode->width;
            height = mode->height;
        }
        else {
            xPos=windowed_window_parameters.xPos;
            yPos= windowed_window_parameters.yPos;
            width =windowed_window_parameters.width ;
            height =windowed_window_parameters.height;
        }

        glfwSetWindowMonitor(window, monitor, xPos, yPos, width, height, GLFW_DONT_CARE);
        set_framebuffer_size({width,height});

        //TODO: Rebuild the swapchain, probably
    }

    bool glfw_vk_window::should_close() {
        return (bool) glfwWindowShouldClose(window);
    }

    glm::ivec2 glfw_vk_window::get_size() {
        return window_dimensions;
    }

    void glfw_vk_window::end_frame() {
        // We're in thread 29
        glfwSwapBuffers(window);
        glfwPollEvents();

        glm::ivec2 new_window_size;
        glfwGetFramebufferSize(window, &new_window_size.x, &new_window_size.y);
		
        if(new_window_size != window_dimensions) {
            set_framebuffer_size(new_window_size);
        }
    }

    void glfw_vk_window::set_framebuffer_size(glm::ivec2 new_framebuffer_size) {
        nlohmann::json &settings = nova_renderer::instance->get_render_settings().get_options();
        settings["settings"]["viewWidth"] = new_framebuffer_size.x;
        settings["settings"]["viewHeight"] = new_framebuffer_size.y;
        window_dimensions = new_framebuffer_size;
        //glViewport(0, 0, window_dimensions.x, window_dimensions.y);
        nova_renderer::instance->get_render_settings().update_config_changed();
    }

    void glfw_vk_window::on_config_change(nlohmann::json &new_config) {
        LOG(INFO) << "gl_glfw_window received the updated config";
    }

    void glfw_vk_window::on_config_loaded(nlohmann::json &config) {
    }

    bool glfw_vk_window::is_active() {
        return active;
    }

    void glfw_vk_window::setActive(bool active) {
        glfw_vk_window::active = active;
    }

    void glfw_vk_window::set_mouse_grabbed(bool grabbed) {
        glfwSetInputMode(window, GLFW_CURSOR, grabbed ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    const char** glfw_vk_window::get_required_extensions(uint32_t* count) const {
        return glfwGetRequiredInstanceExtensions(count);
    }

    void glfw_vk_window::create_surface() {
        auto err = glfwCreateWindowSurface(render_context::instance.vk_instance, window, nullptr, reinterpret_cast<VkSurfaceKHR *>(&render_context::instance.surface));
        if(err != VK_SUCCESS) {
            LOG(FATAL) << "Could not create surface";
        }
    }
}
