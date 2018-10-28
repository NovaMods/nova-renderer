//
// Created by jannis on 30.08.18.
//

#ifndef NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
#define NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP

#include "../render_engine.hpp"
#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR  // Use X11 for window creating on Linux... TODO: Wayland?
#define NOVA_VK_XLIB 1
#elif __win32
#define VK_USE_PLATFORM_WIN32_KHR
#define NOVA_USE_WIN32 1
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

        void open_window(uint32_t width, uint32_t height) override;

        std::shared_ptr<iwindow> get_window() const override;

        void set_shaderpack(const shaderpack_data &data, ftl::TaskScheduler& scheduler) override;

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
        std::vector<VkImage> swapchain_images;
        VkFormat swapchain_format;
        VkExtent2D swapchain_extent;
        std::vector<VkImageView> swapchain_image_views;
        std::vector<VkFramebuffer> swapchain_framebuffers;
        uint32_t current_swapchain_index = 0;

        struct vk_render_pass {
            VkRenderPass vk_pass;
            render_pass_data nova_data;
        };
        std::unordered_map<std::string, vk_render_pass> render_passes_by_name;
        std::vector<std::string> render_passes_by_order;

        struct vk_resource_binding : VkDescriptorSetLayoutBinding {
            uint32_t set;
        };

        struct vk_pipeline {
            VkPipeline vk_pipeline;
            VkPipelineLayout vk_layout;
            pipeline_data nova_data;

            std::unordered_map<std::string, vk_resource_binding> bindings;
        };
        std::unordered_map<std::string, vk_pipeline> pipelines;

        struct vk_texture {
            VkImage vk_image;
            VkImageView vk_image_view;

            texture_resource_data nova_data;

            VmaAllocation vma_allocation;
            VmaAllocationInfo vma_info;
        };
        std::unordered_map<std::string, vk_texture> dynamic_textures_by_name;

        std::unordered_map<std::string, material_data> materials;

        VkCommandPool command_pool;
        std::vector<VkCommandBuffer> command_buffers;

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
        void create_swapchain_image_views();
        void destroy_image_views();
        /*!
         * \brief Creates a Vulkan renderpass for every element in passes
         * \param passes A list of render_pass_infos to create Vulkan renderpasses for
         */
        void create_render_passes(const std::vector<render_pass_data>& passes);
        void destroy_render_passes();
        void create_graphics_pipelines();
        void destroy_graphics_pipelines();
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

        void cleanup_dynamic();  // Cleanup objects that have been created on the fly

        const uint MAX_FRAMES_IN_QUEUE = 3;
        uint current_frame = 0;

        const std::vector<vulkan::vulkan_vertex> verticies = {{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

#ifndef NDEBUG
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
        PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;

        static VkBool32 debug_report_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type,
                                              uint64_t object, size_t location, int32_t messageCode,
                                              const char *layer_prefix, const char *message, void *user_data);

        VkDebugReportCallbackEXT debug_callback;
#endif

        std::tuple<std::vector<VkAttachmentDescription>, std::vector<VkAttachmentReference>>
        to_vk_attachment_info(std::vector<std::string> &attachment_names);

        VkFormat to_vk_format(pixel_format_enum format);

        /*!
         * \brief Adds an entry to the dynamic textures for each entry in texture_data
         * \param texture_datas All the texture_datas that you want to create a dynamic texture for
         */
        void create_textures(const std::vector<texture_resource_data>& texture_datas);

        VkShaderModule create_shader_module(std::vector<uint32_t> spirv);

        void get_attribute_descriptions(std::vector<uint32_t> spirv,
                                        std::unordered_map<std::string, vk_resource_binding>& bindings);

        void process_bindings(std::unordered_map<std::string, vk_resource_binding> bindings,
                          std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> layout_data);
    };
}  // namespace nova

#endif  // NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
