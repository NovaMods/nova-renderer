//
// Created by David on 15-Apr-16.
//

#include <vulkan/vulkan.h>
#include "glfw_vk_window.h"
#include "../../utils/utils.h"

#include <easylogging++.h>
#include <dxgi.h>
#include "../../input/InputHandler.h"
#include "../nova_renderer.h"
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

        //renderdoc_manager = std::make_unique<RenderDocManager>(window, "C:\\Program Files\\RenderDoc\\renderdoc.dll", "capture");
        //LOG(INFO) << "Hooked into RenderDoc";

        glfwGetFramebufferSize(window, &window_dimensions.x, &window_dimensions.y);

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
    }

    bool glfw_vk_window::should_close() {
        return (bool) glfwWindowShouldClose(window);
    }

    glm::vec2 glfw_vk_window::get_size() {
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

    void glfw_vk_window::create_surface(render_device& context) {
        auto err = glfwCreateWindowSurface(context.vk_instance, window, nullptr, &context.surface);
        if(err != VK_SUCCESS) {
            LOG(FATAL) << "Could not create surface";
        }
    }

    void glfw_vk_window::create_swapchain(gpu_info* gpu) {
        auto surface_format = choose_surface_format(gpu->surface_formats);
        auto present_mode = choose_present_mode(gpu->present_modes);
        auto extent = choose_surface_extent(gpu->surface_capabilities);

        VkSwapchainCreateInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = render_device::instance.surface;

        info.minImageCount = NUM_FRAME_DATA;

        info.imageFormat = surface_format.format;
        info.imageColorSpace = surface_format.colorSpace;
        info.imageExtent = extent;
        info.imageArrayLayers = 1;

        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        if(render_device::instance.graphics_family_idx not_eq render_device::instance.present_family_idx) {
            // If the indices are different then we need to share the images
            uint32_t indices[] = {render_device::instance.graphics_family_idx, render_device::instance.present_family_idx};

            info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            info.queueFamilyIndexCount = 2;
            info.pQueueFamilyIndices = indices;
        } else {
            // If the indices are the same, we can have exclusive access
            info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = present_mode;

        info.clipped = VK_TRUE;

        auto err = vkCreateSwapchainKHR(render_device::instance.device, &info, nullptr, &swapchain);
        if(err != VK_SUCCESS) {
            LOG(FATAL) << "Could not create swapchain";
        }
    }

    VkSurfaceFormatKHR glfw_vk_window::choose_surface_format(std::vector<VkSurfaceFormatKHR> &formats) {
        VkSurfaceFormatKHR result = {};

        if(formats.size() == 1 and formats[0].format == VK_FORMAT_UNDEFINED) {
            result.format = VK_FORMAT_B8G8R8A8_UNORM;
            result.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
            return result;
        }

        // We want 32 bit rgba and srgb nonlinear... I think? Will have to read up on it more and figure out what's up
        for(auto& fmt : formats) {
            if(fmt.format == VK_FORMAT_B8G8R8A8_UNORM and fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return fmt;
            }
        }

        // We can't have what we want, so I guess we'll just use what we got
        return formats[0];
    }

    VkPresentModeKHR glfw_vk_window::choose_present_mode(std::vector<VkPresentModeKHR> &modes) {
        const VkPresentModeKHR desired_mode = VK_PRESENT_MODE_MAILBOX_KHR;

        // Mailbox mode is best mode (also not sure why)
        for(auto& mode : modes) {
            if(mode == desired_mode) {
                return desired_mode;
            }
        }

        // FIFO, like FIFA, is forever
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D glfw_vk_window::choose_surface_extent(VkSurfaceCapabilitiesKHR &caps) {
        VkExtent2D extent;

        if(caps.currentExtent.width == -1) {
            extent.width = static_cast<uint32_t>(window_dimensions.x);
            extent.height = static_cast<uint32_t>(window_dimensions.y);
        } else {
            extent = caps.currentExtent;
        }

        return extent;
    }
}
