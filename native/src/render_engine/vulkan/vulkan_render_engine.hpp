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
#include <thread>
#include <mutex>
#include "../render_engine.hpp"
#include "vulkan_utils.hpp"
#include "x11_window.hpp"
#include "../command_buffer_base.hpp"

namespace nova {
    struct vulkan_queue {
        VkQueue queue;
        uint32_t queue_idx;
    };

    class vulkan_render_engine : public render_engine {
    public:
        explicit vulkan_render_engine(const settings &settings);
        ~vulkan_render_engine() override;

        void render_frame() override;

        void set_frame_graph() override;

        void open_window(uint32_t width, uint32_t height) override;

        std::shared_ptr<iwindow> get_window() const override;

        static const std::string get_engine_name();

    private:
        std::vector<const char *> enabled_validation_layer_names;

        VkInstance vk_instance;
#ifdef NOVA_VK_XLIB
        std::shared_ptr<x11_window> window;
#endif
        VkSurfaceKHR surface;
        VkPhysicalDevice physical_device;
        VkDevice device;

        VkSwapchainKHR swapchain;
        VkRenderPass render_pass;
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;

        std::vector<VkImage> swapchain_images;
        VkFormat swapchain_format;
        VkExtent2D swapchain_extend;
        std::vector<VkImageView> swapchain_image_views;
        std::vector<VkFramebuffer> swapchain_framebuffers;
        uint32_t current_swapchain_index = 0;

        VkShaderModule vert_shader;
        VkShaderModule frag_shader;

        VkSemaphore render_finished_semaphore;
        VkSemaphore image_available_semaphore;

        VkQueue graphics_queue;
        VkQueue compute_queue;
        VkQueue copy_queue;

        void create_device();
        void destroy_device();
        bool does_device_support_extensions(VkPhysicalDevice device);
        void create_swapchain();
        void destroy_swapchain();
        void create_image_views();
        void destroy_image_views();
        void create_render_pass();
        void destroy_render_pass();
        void create_graphics_pipeline();
        void destroy_graphics_pipeline();
        void create_framebuffers();
        void destroy_framebuffers();
        void create_semaphores();
        void destroy_semaphores();
        VkSurfaceFormatKHR choose_swapchain_format(const std::vector<VkSurfaceFormatKHR> &available);
        VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR> &available);

        void cleanup_dynamic(); // Cleanup objects that have been created on the fly

        void DEBUG_create_shaders();
        void DEBUG_destroy_shaders();
        std::vector<char> DEBUG_read_file(std::string path);

#ifndef NDEBUG
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
        PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;

        static VkBool32 debug_report_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type, uint64_t object, size_t location, int32_t messageCode,
                const char *layer_prefix, const char *message, void *user_data);

        VkDebugReportCallbackEXT debug_callback;
#endif
    };
}


#endif //NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
