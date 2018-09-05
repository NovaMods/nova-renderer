//
// Created by jannis on 30.08.18.
//

#ifndef NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
#define NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP

#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR // Use X11 for window creating on Linux... TODO: Wayland?
#define NOVA_VK_XLIB
#endif
#include <vulkan/vulkan.h>
#include "../render_engine.hpp"
#include "vulkan_utils.hpp"
#include "x11_window.hpp"

namespace nova {
    class vulkan_render_engine : public render_engine {
    private:
        std::vector<const char *> enabled_validation_layer_names;

        VkInstance vk_instance;
#ifdef NOVA_VK_XLIB
        std::shared_ptr<x11_window> window;
#endif
        VkSurfaceKHR surface;
        VkPhysicalDevice physical_device;
        VkDevice device;
        VkQueue graphics_queue;
        VkSwapchainKHR swapchain;

        std::vector<VkImage> swapchain_images;
        VkFormat swapchain_format;
        VkExtent2D swapchain_extend;

        void create_device();
        void destroy_device();
        bool does_device_support_extensions(VkPhysicalDevice device);
        void create_swapchain();
        void destroy_swapchain();
        VkSurfaceFormatKHR choose_swapchain_format(const std::vector<VkSurfaceFormatKHR> &available);
        VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR> &available);

#ifndef NDEBUG
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
        PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;

        static VkBool32 debug_report_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type, uint64_t object, size_t location, int32_t messageCode,
                const char *layer_prefix, const char *message, void *user_data);

        VkDebugReportCallbackEXT debug_callback;
#endif

    public:
        explicit vulkan_render_engine(const settings &settings);
        ~vulkan_render_engine();

        void open_window(uint32_t width, uint32_t height) override;

        std::shared_ptr<iwindow> get_window() const override;

        std::shared_ptr<iframebuffer> get_current_swapchain_framebuffer() const override;

        std::shared_ptr<iresource> get_current_swapchain_image() const override;

        std::unique_ptr<command_buffer_base> allocate_command_buffer(command_buffer_type type) override;

        void execute_command_buffers(const std::vector<command_buffer_base*>& buffers) override;

        void free_command_buffer(std::unique_ptr<command_buffer_base> buf) override;

        void present_swapchain_image() override;

        static const std::string get_engine_name();
    };
}


#endif //NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
