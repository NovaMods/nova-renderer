/*!
 * \author ddubois 
 * \date 20-Aug-18.
 */

#include "glfw_window.hpp"

#if SUPPORT_VULKAN

#include <vulkan/vulkan.h>

<<<<<<< HEAD:native/src/vulkan/glfw_window.cpp
#include "../util/utils.hpp"
#include "../nova_renderer.hpp"
#include "../util/logger.hpp"
=======
#include "glfw_window.hpp"
#include "../../util/utils.hpp"
#include "../../nova_renderer.hpp"
#include "../../util/logger.hpp"
>>>>>>> 58c0147ec7b2aca0387f1ac95c24b1be62d4b0ab:native/src/render_engine/vulkan/glfw_window.cpp

namespace nova {
    void error_callback(int error, const char *description) {
        // TODO: This message is a bit... bad
        nova::logger::instance.log(nova::log_level::ERROR) << "Error callback: " << error << ": " << description;
    }

    void window_focus_callback(GLFWwindow *window, int focused) {
        glfw_vk_window::setActive((bool) focused);
    }

    bool glfw_vk_window::active = true;

    glfw_vk_window::glfw_vk_window() {
        initialize_logging();

        glfwSetErrorCallback(error_callback);

        if(glfwInit() == 0) {
<<<<<<< HEAD:native/src/vulkan/glfw_window.cpp
            logger::instance.log(log_level::ERROR, "Could not initialize GLFW");
            exit(-1);
=======
            NOVA_LOG(FATAL) << "Could not initialize GLFW";
>>>>>>> 58c0147ec7b2aca0387f1ac95c24b1be62d4b0ab:native/src/render_engine/vulkan/glfw_window.cpp
        }

        nlohmann::json &config = nova_renderer::get_render_settings().get_options();

        float view_width = config["settings"]["viewWidth"];
        float view_height = config["settings"]["viewHeight"];

        if(!glfwVulkanSupported()) {
            NOVA_LOG(FATAL) << "Your system does not support Vulkan. Nova will now exit";
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO: Deal with this later
        window = glfwCreateWindow((int)view_width, (int)view_height, "Minecraft Nova Renderer", nullptr, nullptr);
        if(window == nullptr) {
            NOVA_LOG(FATAL) << "Could not initialize window :(";
        }
        NOVA_LOG(DEBUG) << "GLFW window created";

        glfwGetWindowSize(window, &window_dimensions.x, &window_dimensions.y);

        if(should_enable_renderdoc()) {
#if __WIN32__
            renderdoc_manager = std::make_unique<RenderDocManager>(window,
                                                                   R"(C:\Program Files\RenderDoc\renderdoc.dll)",
                                                                   "captures/nova");
            NOVA_LOG(INFO) << "Hooked into RenderDoc";
#endif
        }

        glfwSetKeyCallback(window, key_callback);
        glfwSetCharCallback(window, key_character_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetCursorPosCallback(window, mouse_position_callback);
        glfwSetScrollCallback(window, mouse_scroll_callback);
        glfwSetWindowFocusCallback(window, window_focus_callback);

        windowed_window_parameters = {};
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

    glm::ivec2& glfw_vk_window::get_size() {
        return window_dimensions;
    }

    void glfw_vk_window::on_frame_end() {
        glfwPollEvents();

        glm::ivec2 new_window_size;
        glfwGetFramebufferSize(window, &new_window_size.x, &new_window_size.y);

        if(new_window_size != window_dimensions) {
            set_framebuffer_size(new_window_size);
        }
    }

    void glfw_vk_window::set_framebuffer_size(glm::ivec2 new_framebuffer_size) {
        nlohmann::json &settings = nova_renderer::get_instance()->get_settings().get_options();
        settings["settings"]["viewWidth"] = new_framebuffer_size.x;
        settings["settings"]["viewHeight"] = new_framebuffer_size.y;
        window_dimensions = new_framebuffer_size;
        nova_renderer::get_instance()->get_settings().update_config_changed();
    }

    void glfw_vk_window::on_config_change(nlohmann::json &new_config) {
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

    void glfw_vk_window::create_surface(vk::Instance instance, vk::SurfaceKHR surface) {
        auto err = glfwCreateWindowSurface((VkInstance)instance, window, nullptr, reinterpret_cast<VkSurfaceKHR *>(&surface));
        if(err != VK_SUCCESS) {
            NOVA_LOG(FATAL) << "Could not create surface";
        }
    }

    bool glfw_vk_window::should_enable_renderdoc() {
        auto& settings = nova_renderer::get_instance()->get_settings();

        auto static_settings = get_json_value<nlohmann::json>(settings.get_options(), "readOnly");
        if(static_settings) {
            auto should_enable_renderdoc = get_json_value<bool>(*static_settings, "enableRenderDoc");
            if(*should_enable_renderdoc) {
                return true;
            }
        }

        return false;
    }
}
#endif