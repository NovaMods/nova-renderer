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

        // glfwGetFramebufferSize(window, &window_dimensions.x, &window_dimensions.y);

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

    void glfw_vk_window::create_surface() {
        VkSurfaceKHR lame_surface;
        auto err = glfwCreateWindowSurface(render_device::instance.vk_instance, window, nullptr, &lame_surface);
        if(err != VK_SUCCESS) {
            LOG(FATAL) << "Could not create surface";
        }
        render_device::instance.surface = lame_surface;
    }

    void glfw_vk_window::create_swapchain(gpu_info& gpu) {
        auto& device = render_device::instance.device;

        auto surface_format = choose_surface_format(gpu.surface_formats);
        auto present_mode = choose_present_mode(gpu.present_modes);
        auto extent = choose_surface_extent(gpu.surface_capabilities);

        vk::SwapchainCreateInfoKHR info = {};
        info.surface = render_device::instance.surface;

        info.minImageCount = NUM_FRAME_DATA;

        info.imageFormat = surface_format.format;
        info.imageColorSpace = surface_format.colorSpace;
        info.imageExtent = extent;
        info.imageArrayLayers = 1;

        info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;

        if(render_device::instance.graphics_family_idx not_eq render_device::instance.present_family_idx) {
            // If the indices are different then we need to share the images
            uint32_t indices[] = {render_device::instance.graphics_family_idx, render_device::instance.present_family_idx};

            info.imageSharingMode = vk::SharingMode::eConcurrent;
            info.queueFamilyIndexCount = 2;
            info.pQueueFamilyIndices = indices;
        } else {
            // If the indices are the same, we can have exclusive access
            info.imageSharingMode = vk::SharingMode::eExclusive;
        }

        info.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
        info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        info.presentMode = present_mode;

        info.clipped = VK_TRUE;

        swapchain = device.createSwapchainKHR(info);

        swapchain_format = surface_format.format;
        this->present_mode = present_mode;
        swapchain_extent = extent;

        std::vector<vk::Image> swapchain_images = device.getSwapchainImagesKHR(swapchain);
        if(swapchain_images.empty()) {
            LOG(FATAL) << "The swapchain returned zero images";
        }

        // More than 255 images in the swapchain? Good lord what are you doing? and will you please stop?
        for(uint8_t i = 0; i < NUM_FRAME_DATA; i++) {
            vk::ImageViewCreateInfo image_view_create_info = {};

            image_view_create_info.image = swapchain_images[i];
            image_view_create_info.viewType = vk::ImageViewType::e2D;
            image_view_create_info.format = swapchain_format;

            image_view_create_info.components.r = vk::ComponentSwizzle::eR;
            image_view_create_info.components.g = vk::ComponentSwizzle::eG;
            image_view_create_info.components.b = vk::ComponentSwizzle::eB;
            image_view_create_info.components.a = vk::ComponentSwizzle::eA;

            image_view_create_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;

            vk::ImageView image_view = device.createImageView(image_view_create_info);

            this->swapchain_images.push_back(image_view);
        }
    }

    vk::SurfaceFormatKHR glfw_vk_window::choose_surface_format(std::vector<vk::SurfaceFormatKHR> &formats) {
        vk::SurfaceFormatKHR result = {};

        if(formats.size() == 1 and formats[0].format == vk::Format::eUndefined) {
            result.format = vk::Format::eB8G8R8A8Unorm;
            result.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
            return result;
        }

        // We want 32 bit rgba and srgb nonlinear... I think? Will have to read up on it more and figure out what's up
        for(auto& fmt : formats) {
            if(fmt.format == vk::Format::eB8G8R8A8Unorm and fmt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return fmt;
            }
        }

        // We can't have what we want, so I guess we'll just use what we got
        return formats[0];
    }

    vk::PresentModeKHR glfw_vk_window::choose_present_mode(std::vector<vk::PresentModeKHR> &modes) {
        const vk::PresentModeKHR desired_mode = vk::PresentModeKHR::eMailbox;

        // Mailbox mode is best mode (also not sure why)
        for(auto& mode : modes) {
            if(mode == desired_mode) {
                return desired_mode;
            }
        }

        // FIFO, like FIFA, is forever
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D glfw_vk_window::choose_surface_extent(vk::SurfaceCapabilitiesKHR &caps) {
        vk::Extent2D extent;

        if(caps.currentExtent.width == -1) {
            extent.width = static_cast<uint32_t>(window_dimensions.x);
            extent.height = static_cast<uint32_t>(window_dimensions.y);
        } else {
            extent = caps.currentExtent;
        }

        return extent;
    }
}
