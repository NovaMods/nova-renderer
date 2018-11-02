//
// Created by jannis on 30.08.18.
//

#ifndef NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
#define NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP

#include "../render_engine.hpp"
#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR  // Use X11 for window creating on Linux... TODO: Wayland?
#define NOVA_VK_XLIB 1
#elif _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define NOVA_USE_WIN32 1
#endif
#include <vulkan/vulkan.h>
#include <thread>
#include <mutex>
#include "vulkan_utils.hpp"
#include "x11_window.hpp"

#include <vk_mem_alloc.h>
#include "../dx12/win32_window.hpp"

namespace nova {
    struct vulkan_queue {
        VkQueue queue;
        uint32_t queue_idx;
    };

    struct vk_resource_binding : VkDescriptorSetLayoutBinding {
        uint32_t set;

        bool operator==(const vk_resource_binding& other) const;
        bool operator!=(const vk_resource_binding& other) const;
    };

    struct vk_pipeline {
        VkPipeline pipeline;
        VkPipelineLayout layout;
        pipeline_data nova_data;

        std::unordered_map<std::string, vk_resource_binding> bindings;
    };

    struct vk_texture {
        VkImage image;
        VkImageView image_view;

        texture_resource_data nova_data;

        VmaAllocation allocation;
        VmaAllocationInfo vma_info;
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
#ifdef linux
        std::shared_ptr<x11_window> window;
#elif _WIN32
        std::shared_ptr<win32_window> window;
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
            VkRenderPass pass;
            render_pass_data nova_data;
        };
        std::unordered_map<std::string, vk_render_pass> render_passes_by_name;
        std::vector<std::string> render_passes_by_order;

        std::unordered_map<std::string, vk_pipeline> pipelines;

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
        static bool does_device_support_extensions(VkPhysicalDevice device);
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
        void create_graphics_pipelines(const std::vector<pipeline_data>& pipelines);
        void destroy_graphics_pipelines();
        void create_command_pool();
        void destroy_command_pool();
        void create_command_buffers();
        void create_synchronization_objects();
        void destroy_synchronization_objects();
        static VkSurfaceFormatKHR choose_swapchain_format(const std::vector<VkSurfaceFormatKHR> &available);
        static VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR> &available);
        VkExtent2D choose_swapchain_extend() const;
        void recreate_swapchain();

        void cleanup_dynamic();  // Cleanup objects that have been created on the fly

        const uint MAX_FRAMES_IN_QUEUE = 3;
        uint current_frame = 0;

#ifndef NDEBUG
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
        PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
        
        VkDebugReportCallbackEXT debug_callback;
#endif

        std::pair<std::vector<VkAttachmentDescription>, std::vector<VkAttachmentReference>>
        to_vk_attachment_info(std::vector<std::string> &attachment_names);

        static VkFormat to_vk_format(pixel_format_enum format);

        /*!
         * \brief Adds an entry to the dynamic textures for each entry in texture_data
         * \param texture_datas All the texture_datas that you want to create a dynamic texture for
         */
        void create_textures(const std::vector<texture_resource_data>& texture_datas);

        VkShaderModule create_shader_module(std::vector<uint32_t> spirv) const;

        /*!
         * \brief Gets all the descriptor bindings from the provided SPIR-V code, performing basic validation that the 
         * user hasn't declared two different bindings with the same name
         * 
         * \param spirv The SPIR-V shader code to get bindings from
         * \param bindings An in/out array that holds all the existing binding before this method, and holds the 
         * existing bindings plus new ones declared in the shader after this method
         */
        static void get_shader_module_descriptors(std::vector<uint32_t> spirv,
                                        std::unordered_map<std::string, vk_resource_binding>& bindings);

        /*!
         * \brief Creates descriptor set layouts for all the descriptor set bindings 
         * 
         * \param bindings All the bindings we know about
         * \return A list of descriptor set layouts, one for each set in `bindings`
         */
        std::vector<VkDescriptorSetLayout> create_descriptor_set_layouts(std::unordered_map<std::string, vk_resource_binding> bindings) const;
    };

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(VkDebugReportFlagsEXT flags, 
        VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t message_code, 
        const char *layer_prefix, const char *message, void *user_data);
}  // namespace nova

#endif  // NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
