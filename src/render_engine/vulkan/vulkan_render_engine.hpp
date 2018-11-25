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
#include "aligned_block_allocator.hpp"
#include "spirv_glsl.hpp"

#include "ftl/atomic_counter.h"
#include "ftl/fibtex.h"
#include "ftl/task_scheduler.h"
#include "ftl/thread_local.h"

namespace nova {
    struct vk_queue {
        VkQueue queue = VK_NULL_HANDLE;
        uint32_t queue_idx;
    };

    struct vk_resource_binding : VkDescriptorSetLayoutBinding {
        uint32_t set;

        bool operator==(const vk_resource_binding& other) const;
        bool operator!=(const vk_resource_binding& other) const;
    };

    struct vk_render_pass {
        VkRenderPass pass = VK_NULL_HANDLE;
        render_pass_data data;
    };

    struct vk_pipeline {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;
        pipeline_data data;

        std::unordered_map<std::string, vk_resource_binding> bindings;
    };

    struct vk_texture {
        VkImage image = VK_NULL_HANDLE;
        VkImageView image_view = VK_NULL_HANDLE;

        texture_resource_data data;

        VmaAllocation allocation;
        VmaAllocationInfo vma_info;
    };

    struct vk_buffer {
        VkBuffer buffer = VK_NULL_HANDLE;

        VmaAllocation allocation;
        VmaAllocationInfo alloc_info;
    };

    struct staging_buffer_upload_command {
        std::vector<vk_buffer> staging_buffers;
        aligned_block_allocator<sizeof(full_vertex)>::allocation mem;
        uint32_t mesh_id;
    };

    struct vk_mesh {
        aligned_block_allocator<sizeof(full_vertex)>::allocation vertex_memory;
    };

    class vulkan_render_engine : public render_engine {
    public:
        explicit vulkan_render_engine(const nova_settings& settings, ftl::TaskScheduler* task_scheduler);
        ~vulkan_render_engine() override;

        void render_frame() override;

        void open_window(uint32_t width, uint32_t height) override;

        std::shared_ptr<iwindow> get_window() const override;

        void set_shaderpack(const shaderpack_data& data) override;

        uint32_t add_mesh(const mesh_data& input_mesh) override;

        void delete_mesh(uint32_t mesh_id) override;

    private:
        const uint MAX_FRAMES_IN_QUEUE = 3;
        uint current_frame = 0;

        std::vector<const char*> enabled_validation_layer_names;

#ifdef linux
        std::shared_ptr<x11_window> window;
#elif _WIN32
        std::shared_ptr<win32_window> window;
#endif

#pragma region Globals
        VkInstance vk_instance;

        VkSurfaceKHR surface;
        VkPhysicalDevice physical_device;
        VkDevice device;

        VmaAllocator vma_allocator;

        std::vector<VkSemaphore> render_finished_semaphores;
        std::vector<VkSemaphore> image_available_semaphores;
        std::vector<VkFence> submit_fences;

        /*!
         * \brief Thread-local command pools so multiple tasks don't try to use the same command pools at the same time
         */
        ftl::ThreadLocal<std::unordered_map<uint32_t, VkCommandPool>> command_pools_by_queue_idx;

        /*!
         * \brief Allocates new command pools - one for graphics, one for transfer, one for compute
         */
        std::unordered_map<uint32_t, VkCommandPool> make_new_command_pools() const;

        /*!
         * \brief Retrieves the command pool for the current thread, or creates a new one if there is nothing or the
         * current thread
         *
         * \param queue_index the index of the queue we need to get a command pool for
         *
         * \return The command pool for the current thread
         */
        VkCommandPool get_command_buffer_pool_for_current_thread(uint32_t queue_index);
#pragma endregion

#pragma region Init
        static bool does_device_support_extensions(VkPhysicalDevice device);
        static VkSurfaceFormatKHR choose_swapchain_format(const std::vector<VkSurfaceFormatKHR>& available);
        static VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& available);
        VkExtent2D choose_swapchain_extend() const;

        void create_device();
        void create_memory_allocator();
        void create_swapchain();
        void create_swapchain_image_views();
#pragma endregion

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

        std::unordered_map<std::string, vk_render_pass> render_passes;
        std::vector<std::string> render_passes_by_order;

        std::unordered_map<std::string, vk_pipeline> pipelines;

        std::unordered_map<std::string, vk_texture> dynamic_textures;

        std::unordered_map<std::string, material_data> materials;

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
         * \brief Adds information about the provided resource to `bindings`
         * 
         * If there's nothing at the descriptor set and binding of `resource`, `resource` is added directly to `bindings`. 
         * If something already exists, `bindings` is unchanged
         * 
         * \param bindings The map of bindings to ass data to
         * \param shader_compiler The compiler used for the shader that the resource came from
         * \param resource The resource to maybe add to `bindings
         */
        static void add_resource_to_bindings(std::unordered_map<std::string, vk_resource_binding>& bindings, 
			const spirv_cross::CompilerGLSL& shader_compiler, const spirv_cross::Resource& resource);

        /*!
         * \brief Creates a Vulkan renderpass for every element in passes
         * \param passes A list of render_pass_infos to create Vulkan renderpasses for
         */
        void create_render_passes(const std::vector<render_pass_data>& passes);

        /*!
         * \brief Creates a VkGraphicsPipeline for each pipeline_data in pipelines
         * \param pipelines The pipeline_datas to create pipelines for
         */
        void create_graphics_pipelines(const std::vector<pipeline_data>& pipelines);

        /*!
         * \brief Creates a single shader module from the provided SPIR-V code
         *
         * \param spirv The SPIR-V to generate the shader module from
         *
         * \return The generated shader module
         */
        VkShaderModule create_shader_module(const std::vector<uint32_t>& spirv) const;

        /*!
         * \brief Adds an entry to the dynamic textures for each entry in texture_data
         * \param texture_datas All the texture_datas that you want to create a dynamic texture for
         */
        void create_textures(const std::vector<texture_resource_data>& texture_datas);

        /*!
         * \brief Creates descriptor set layouts for all the descriptor set bindings
         *
         * \param all_bindings All the bindings we know about
         * \return A list of descriptor set layouts, one for each set in `bindings`
         */
        std::vector<VkDescriptorSetLayout> create_descriptor_set_layouts(std::unordered_map<std::string, vk_resource_binding> all_bindings) const;

        /*!
         * \brief Converts the list of attachment names into attachment descriptions and references that can be later 
         * used to make a VkRenderpass
         * 
         * \param attachment_names The names of the attachments we want to convert
         * \return VkAttachmentDescriptions and VkAttachmentReferences for the attachments we want to convert
         */
        std::pair<std::vector<VkAttachmentDescription>, std::vector<VkAttachmentReference>> to_vk_attachment_info(std::vector<std::string>& attachment_names);

        /*!
         * \brief Destroys all the render passes in `render_passes`
         */
        void destroy_render_passes();

        /*!
         * \brief Destroys all the pipelines in `pipelines`
         */
        void destroy_graphics_pipelines();

        /*!
         * \brief Destroys all the textures in `dynamic_textures`
         */
        void destroy_dynamic_textures();
#pragma endregion

#pragma region Mesh
        std::shared_ptr<aligned_block_allocator<sizeof(full_vertex)>> vertex_memory;
        std::shared_ptr<aligned_block_allocator<sizeof(uint32_t)>> index_memory;

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

        // Might need to make 64-bit keys eventually, but in 2018 it's not a concern
        std::unordered_map<uint32_t, vk_mesh> meshes;
        std::atomic<uint32_t> next_mesh_id = 0;

        /*!
         * \brief Validates that the sizes in `options` are properly aligned
         *
         * `options.buffer_part_size` must be a whole-number multiple of `sizeof(full_vertex)`
         * `options.new_buffer_size` must be a whole-number multiple of `options.buffer_part_size`
         * `options.max_total_allocation` must be a whole-number multiple of `options.new_buffer_size`
         */
        void validate_mesh_options(const settings_options::block_allocator_options& options) const;

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

        /*!
         * \brief If a mesh staging buffer is available, it's returned to the user. Otherwise, a new mesh staging
         * buffer is created - and then returned to the user
         */
        vk_buffer get_or_allocate_mesh_staging_buffer();

        /*!
         * \brief Returns the provided buffer to the pool of staging buffers
         */
        void free_mesh_staging_buffer(const vk_buffer& buffer);
#pragma endregion
        
#pragma region Queues
        uint32_t graphics_queue_index;
        VkQueue graphics_queue;
        uint32_t compute_queue_index;
        VkQueue compute_queue;
        uint32_t copy_queue_index;
        VkQueue copy_queue;
#pragma endregion
        
#ifndef NDEBUG
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
        PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;

        VkDebugReportCallbackEXT debug_callback;
#endif

        static VkFormat to_vk_format(pixel_format_enum format);
    };

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(
        VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t message_code, const char* layer_prefix, const char* message, void* user_data);
}  // namespace nova

#endif  // NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
