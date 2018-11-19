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

#include <vk_mem_alloc.h>
#include <queue>
#include "../dx12/win32_window.hpp"
#include "mesh_allocator.hpp"
#include "spirv_glsl.hpp"

#include "ftl/atomic_counter.h"
#include "ftl/fibtex.h"
#include "ftl/task_scheduler.h"
#include "ftl/thread_local.h"

namespace nova {
    struct vk_queue {
        VkQueue queue;
        uint32_t queue_idx;
    };

    struct vk_resource_binding : VkDescriptorSetLayoutBinding {
        uint32_t set;

        bool operator==(const vk_resource_binding& other) const;
        bool operator!=(const vk_resource_binding& other) const;
    };

    struct vk_render_pass {
        VkRenderPass pass;
        render_pass_data data;
    };

    struct vk_pipeline {
        VkPipeline pipeline;
        VkPipelineLayout layout;
        pipeline_data data;

        std::unordered_map<std::string, vk_resource_binding> bindings;
    };

    struct vk_texture {
        VkImage image;
        VkImageView image_view;

        texture_resource_data data;

        VmaAllocation allocation;
        VmaAllocationInfo vma_info;
    };

    struct vk_buffer {
        VkBuffer buffer;

        VmaAllocation allocation;
        VmaAllocationInfo alloc_info;
    };

    struct staging_buffer_upload_command {
        std::vector<vk_buffer> staging_buffers;
        mesh_memory mem;
    };

    struct vk_mesh {
        mesh_memory memory;
        mesh_data data;
    };

    class vulkan_render_engine : public render_engine {
    public:
        explicit vulkan_render_engine(const nova_settings& settings, ftl::TaskScheduler* task_scheduler);
        ~vulkan_render_engine() override;

        void render_frame() override;

        void open_window(uint32_t width, uint32_t height) override;

        std::shared_ptr<iwindow> get_window() const override;

        void set_shaderpack(const shaderpack_data& data) override;

        uint32_t add_mesh(const mesh_data& mesh) override;

        void delete_mesh(uint32_t mesh_id) override;

    private:
        std::vector<const char*> enabled_validation_layer_names;

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

#pragma region Swapchain
        VkSwapchainKHR swapchain;
        std::vector<VkImage> swapchain_images;
        VkFormat swapchain_format;
        VkExtent2D swapchain_extent;
        std::vector<VkImageView> swapchain_image_views;
        std::vector<VkFramebuffer> swapchain_framebuffers;
        uint32_t current_swapchain_index = 0;
#pragma endregion

#pragma region Shaderpack
        bool shaderpack_loaded = false;
        shaderpack_data shaderpack;

        std::unordered_map<std::string, vk_render_pass> render_passes_by_name;
        std::vector<std::string> render_passes_by_order;

        std::unordered_map<std::string, vk_pipeline> pipelines;

        std::unordered_map<std::string, vk_texture> dynamic_textures_by_name;

        std::unordered_map<std::string, material_data> materials;
#pragma endregion

#pragma region Mesh
        std::shared_ptr<mesh_allocator> mesh_manager;
        /*!
         * \brief The number of mesh upload tasks that are still running
         */
        ftl::AtomicCounter upload_to_staging_buffers_counter;
        ftl::Fibtex mesh_staging_buffers_mutex;
        std::vector<vk_buffer> mesh_staging_buffers;

        std::queue<staging_buffer_upload_command> mesh_upload_queue;
        ftl::Fibtex mesh_upload_queue_mutex;
        VkFence mesh_rendering_done;
        VkFence upload_to_megamesh_buffer_done;
#pragma endregion

        ftl::ThreadLocal<std::unordered_map<uint32_t, VkCommandPool>> command_pools_by_queue_idx;

        std::vector<VkSemaphore> render_finished_semaphores;
        std::vector<VkSemaphore> image_available_semaphores;
        std::vector<VkFence> submit_fences;

        uint32_t graphics_queue_index;
        VkQueue graphics_queue;
        uint32_t compute_queue_index;
        VkQueue compute_queue;
        uint32_t copy_queue_index;
        VkQueue copy_queue;
        
        void validate_mesh_options(const settings_options::mesh_options& options) const;

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
        void create_synchronization_objects();
        void destroy_synchronization_objects();
        static VkSurfaceFormatKHR choose_swapchain_format(const std::vector<VkSurfaceFormatKHR>& available);
        static VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& available);
        VkExtent2D choose_swapchain_extend() const;
        void recreate_swapchain();

        void cleanup_dynamic();  // Cleanup objects that have been created on the fly

        /*!
         * \brief Records and submits a command buffer that barriers until reading vertex data from the megamesh
         * buffer has finished, uploads new mesh parts, then barriers until transfers to the megamesh vertex buffer
         * are finished
         *
         * The command buffer will wait on fence `mesh_rendering_done` before getting submitted, and it'll signal
         * `upload_to_megamesh_buffer_done` when it's done copying data into the megamesh buffer. This method uploads
         * all new mesh data and it's awesome
         */
        void upload_new_mesh_parts();

        const uint MAX_FRAMES_IN_QUEUE = 3;
        uint current_frame = 0;

#ifndef NDEBUG
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
        PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;

        VkDebugReportCallbackEXT debug_callback;
#endif

        std::pair<std::vector<VkAttachmentDescription>, std::vector<VkAttachmentReference>> to_vk_attachment_info(std::vector<std::string>& attachment_names);

        static VkFormat to_vk_format(pixel_format_enum format);

        std::unordered_map<uint32_t, VkCommandPool> make_new_command_pools() const;

        /*!
         * \brief Adds an entry to the dynamic textures for each entry in texture_data
         * \param texture_datas All the texture_datas that you want to create a dynamic texture for
         */
        void create_textures(const std::vector<texture_resource_data>& texture_datas);
        static void add_resource_to_bindings(std::unordered_map<std::string, vk_resource_binding>& bindings, const spirv_cross::CompilerGLSL& shader_compiler, const spirv_cross::Resource& resource);

        VkShaderModule create_shader_module(std::vector<uint32_t> spirv) const;

        /*!
         * \brief Gets all the descriptor bindings from the provided SPIR-V code, performing basic validation that the
         * user hasn't declared two different bindings with the same name
         *
         * \param spirv The SPIR-V shader code to get bindings from
         * \param bindings An in/out array that holds all the existing binding before this method, and holds the
         * existing bindings plus new ones declared in the shader after this method
         */
        static void get_shader_module_descriptors(std::vector<uint32_t> spirv, std::unordered_map<std::string, vk_resource_binding>& bindings);

        /*!
         * \brief Creates descriptor set layouts for all the descriptor set bindings
         *
         * \param bindings All the bindings we know about
         * \return A list of descriptor set layouts, one for each set in `bindings`
         */
        std::vector<VkDescriptorSetLayout> create_descriptor_set_layouts(std::unordered_map<std::string, vk_resource_binding> bindings) const;

        /*!
         * \brief Retrieves the command pool for the current thread, or creates a new one if there is nothing or the
         * current thread
         *
         * \param queue_index the index of the queue we need to get a command pool for
         *
         * \return The command pool for the current thread
         */
        VkCommandPool get_command_buffer_pool_for_current_thread(uint32_t queue_index);

        /*!
         * \brief If a mesh staging buffer is available, it's returned to the user. Otherwise, a new mesh staging
         * buffer is created - and then returned to the user
         */
        vk_buffer get_or_allocate_mesh_staging_buffer();

        /*!
         * \brief Returns the provided buffer to the pool of staging buffers
         */
        void free_mesh_staging_buffer(const vk_buffer& buffer);
    };

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(
        VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t message_code, const char* layer_prefix, const char* message, void* user_data);
}  // namespace nova

#endif  // NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
