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

        void open_window(uint32_t width, uint32_t height) override;

        std::shared_ptr<iwindow> get_window() const override;

        std::shared_ptr<iframebuffer> get_current_swapchain_framebuffer(uint32_t frame_index) const override;

        uint32_t get_current_swapchain_index() const override;

        std::shared_ptr<iresource> get_current_swapchain_image(uint32_t frame_index) const override;

        std::unique_ptr<command_buffer_base> allocate_command_buffer(command_buffer_type type) override;

        void execute_command_buffers(const std::vector<command_buffer_base*>& buffers) override;

        void free_command_buffer(std::unique_ptr<command_buffer_base> buf) override;

        void present_swapchain_image() override;

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

        VkShaderModule vert_shader;
        VkShaderModule frag_shader;

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
        VkSurfaceFormatKHR choose_swapchain_format(const std::vector<VkSurfaceFormatKHR> &available);
        VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR> &available);

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

        /*!
         * \brief A CommandPool can't be used from more than one thread at once, so we need to figure out what thread
         * the code requesting a command buffer is in and use the appropriate thread pool
         */
        std::unordered_map<std::thread::id, VkCommandPool> thread_local_pools;
        std::mutex thread_local_pools_lock;

        /*!
         * \brief Same as above - a command buffer is tied to a command pool, so they need to be used in the same thread
         */
        std::unordered_map<std::thread::id, std::unordered_map<command_buffer_type, std::vector<std::unique_ptr<command_buffer_base>>>> thread_local_buffers;
        std::mutex thread_local_buffers_lock;

        /*!
         * \brief The queue that supports the operations that each command buffer type needs
         */
        std::unordered_map<command_buffer_type, vulkan_queue> queues_per_type;
    };
}


#endif //NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
