//
// Created by jannis on 30.08.18.
//

#ifndef NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
#define NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP

#include "../render_engine.hpp"
#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR // Use X11 for window creating on Linux... TODO: Wayland?
#define NOVA_VK_XLIB
#endif
#include <vulkan/vulkan.h>
#include <thread>
#include <mutex>
#include "vulkan_utils.hpp"
#include "x11_window.hpp"

#include <vk_mem_alloc.h>

namespace nova {
    struct vulkan_queue {
        VkQueue queue;
        uint32_t queue_idx;
    };

    class vulkan_render_engine : public render_engine {
    public:
        explicit vulkan_render_engine(const nova_settings &settings);
        ~vulkan_render_engine() override;

        void render_frame() override;

        void set_frame_graph() override;

        void open_window(uint32_t width, uint32_t height) override;

        std::shared_ptr<iwindow> get_window() const override;

        static const std::string get_engine_name();

        void load_shaderpack(shaderpack_data data) override;

    private:
        std::vector<const char *> enabled_validation_layer_names;

        VkInstance vk_instance;
#ifdef NOVA_VK_XLIB
        std::shared_ptr<x11_window> window;
#endif
        VkSurfaceKHR surface;
        VkPhysicalDevice physical_device;
        VkDevice device;

        VmaAllocator memory_allocator;

        VkSwapchainKHR swapchain;
        VkRenderPass render_pass;
        VkPipelineLayout pipeline_layout;
        VkPipeline graphics_pipeline;

        std::vector<VkImage> swapchain_images;
        VkFormat swapchain_format;
        VkExtent2D swapchain_extend;
        std::vector<VkImageView> swapchain_image_views;
        std::vector<VkFramebuffer> swapchain_framebuffers;
        uint32_t current_swapchain_index = 0;
        VkCommandPool command_pool;
        std::vector<VkCommandBuffer> command_buffers;

        VkShaderModule vert_shader = VK_NULL_HANDLE;
        VkShaderModule frag_shader = VK_NULL_HANDLE;

        std::vector<VkSemaphore> render_finished_semaphores;
        std::vector<VkSemaphore> image_available_semaphores;
        std::vector<VkFence> submit_fences;

        uint32_t graphics_queue_index;
        VkQueue graphics_queue;
        uint32_t compute_queue_index;
        VkQueue compute_queue;
        uint32_t copy_queue_index;
        VkQueue copy_queue;

        VkBuffer vertex_buffer;
        VmaAllocation vertex_buffer_allocation;

        bool shaderpack_loaded = false;
        shaderpack_data shaderpack;

        void create_device();
        void destroy_device();
        void create_memory_allocator();
        void destroy_memory_allocator();
        bool does_device_support_extensions(VkPhysicalDevice device);
        void create_swapchain();
        void destroy_swapchain();
        void create_image_views();
        void destroy_image_views();
        void create_render_pass();
        void destroy_render_pass();
        void create_shader_modules();
        void destroy_shader_modules();
        void create_graphics_pipeline();
        void destroy_graphics_pipeline();
        void create_framebuffers();
        void destroy_framebuffers();
        void create_command_pool();
        void destroy_command_pool();
        void create_vertex_buffer();
        void destroy_vertex_buffer();
        void create_command_buffers();
        void create_synchronization_objects();
        void destroy_synchronization_objects();
        VkSurfaceFormatKHR choose_swapchain_format(const std::vector<VkSurfaceFormatKHR> &available);
        VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR> &available);
        VkExtent2D choose_swapchain_extend();
        void recreate_swapchain();

        void cleanup_dynamic(); // Cleanup objects that have been created on the fly

        void DEBUG_record_command_buffers();

        const uint MAX_FRAMES_IN_QUEUE = 3;
        uint current_frame = 0;

        const std::vector<vulkan::vulkan_vertex> verticies = {
                {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };

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
