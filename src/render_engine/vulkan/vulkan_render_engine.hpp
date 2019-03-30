//
// Created by jannis on 30.08.18.
//

#ifndef NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
#define NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP

#include "../render_engine.hpp"
#ifdef NOVA_LINUX
#define VK_USE_PLATFORM_XLIB_KHR // Use X11 for window creating on Linux... TODO: Wayland?
#define NOVA_VK_XLIB
#include "x11_window.hpp"
#include <vulkan/vulkan_xlib.h>
#include "../../util/linux_utils.hpp"

#elif defined(NOVA_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#define NOVA_USE_WIN32
#include "../../util/windows.hpp"
#include <vulkan/vulkan_win32.h>
#include "../dx12/win32_window.hpp"
#endif

#include <vulkan/vulkan.h>

#include <queue>
#include <spirv_cross/spirv_glsl.hpp>
#include "../../util/vma_usage.hpp"
#include "compacting_block_allocator.hpp"

#include <mutex>
#include "../../render_objects/renderables.hpp"
#include "../../render_objects/uniform_structs.hpp"
#include "auto_allocating_buffer.hpp"
#include "fixed_size_buffer_allocator.hpp"
#include "struct_uniform_buffer.hpp"
#include "swapchain.hpp"

#include "../../debugging/renderdoc_app.h"

namespace nova::ttl {
    class task_scheduler;
} // namespace nova::ttl

namespace nova::renderer {

    NOVA_EXCEPTION(buffer_allocate_failed);
    NOVA_EXCEPTION(shaderpack_loading_error);
    NOVA_EXCEPTION(descriptor_pool_creation_failed);

    struct vk_queue {
        VkQueue queue = nullptr;
        uint32_t queue_idx{};
    };

    struct vk_resource_binding : VkDescriptorSetLayoutBinding {
        uint32_t set;

        /*!
         * \brief Checks if two `vk_resource_binding` objects are equal
         *
         * Checks every member except the pipeline stages that the descriptor is available to. This is so I can more
         * easily handle descriptors that are used in multiple pipeline stages
         */
        bool operator==(const vk_resource_binding& other) const;

        /*!
         * \brief Checks if two `vk_resource_binding` objects are not equal
         *
         * Checks every member except the pipeline stages that the descriptor is available to. This is so I can more
         * easily handle descriptors that are used in multiple pipeline stages
         */
        bool operator!=(const vk_resource_binding& other) const;
    };

    struct vk_texture {
        VkImage image = nullptr;
        VkImageView image_view = nullptr;

        texture_resource_data data;

        VmaAllocation allocation{};
        VmaAllocationInfo vma_info{};

        bool is_dynamic = false;
        VkFormat format{};
        VkImageLayout layout{};
        bool is_depth_tex = false;
    };

    struct vk_framebuffer {
        VkFramebuffer framebuffer{};

        std::vector<vk_texture*> images;
    };

    struct vk_render_pass {
        VkRenderPass pass = VK_NULL_HANDLE;

        /*!
         * \brief The framebuffer to use for this render pass
         *
         * If this is VK_NULL_HANDLE, we should use the backbuffer's framebuffer
         */
        vk_framebuffer framebuffer;
        render_pass_data data;
        VkRect2D render_area{};
        VkFence fence{};

        /*!
         * \brief The barriers that should be applied before this pass
         *
         * When a shaderpack is loaded, Nova inspects the resources used by each renderpass. When renderpass B depends
         * on renderpass A, and renderpass A writes to a texture that renderpass B reads from, Nova adds a barrier for
         * that transition. Nova will add a barrier for the opposite situation as well, so that you don't get
         * read-after-write errors
         */
        std::vector<VkImageMemoryBarrier> read_texture_barriers;
        /*!
         * \brief The barriers that should be applied before this pass
         *
         * When a shaderpack is loaded, Nova inspects the resources used by each renderpass. When renderpass B depends
         * on renderpass A, and renderpass A writes to a texture that renderpass B reads from, Nova adds a barrier for
         * that transition. Nova will add a barrier for the opposite situation as well, so that you don't get
         * read-after-write errors
         */
        std::vector<VkImageMemoryBarrier> write_texture_barriers;

        std::optional<VkImageMemoryBarrier> depth_buffer_barrier;

        bool writes_to_backbuffer = false;
    };

    struct vk_pipeline {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;

        std::vector<VkDescriptorSetLayout> layouts;
        pipeline_data data;

        std::unordered_map<std::string, vk_resource_binding> bindings;
    };

    struct vk_buffer {
        VkBuffer buffer = VK_NULL_HANDLE;

        VmaAllocation allocation;
        VmaAllocationInfo alloc_info;

        bool is_dynamic;

        std::string name;
    };

    struct mesh_staging_buffer_upload_command {
        vk_buffer staging_buffer;
        uint32_t mesh_id{};
        uint32_t indices_offset{};
        uint32_t model_matrix_offset{};
    };

    struct vk_mesh {
        vk_buffer index_buffer = {};
        vk_buffer vertex_buffer = {};

        uint32_t num_indices = 0;
        std::size_t num_vertices = 0;

        mesh_id_t id;
    };

    struct vk_gpu_info {
        VkPhysicalDevice phys_device{};
        std::vector<VkQueueFamilyProperties> queue_family_props;
        std::vector<VkExtensionProperties> available_extensions;
        VkSurfaceCapabilitiesKHR surface_capabilities{};
        std::vector<VkSurfaceFormatKHR> surface_formats;
        VkPhysicalDeviceProperties props{};
        VkPhysicalDeviceFeatures supported_features{};
    };

    struct vk_renderables {
        std::unordered_map<mesh_id_t, std::vector<vk_static_mesh_renderable>> static_meshes;
    };

    class vulkan_render_engine : public render_engine {
    public:
        VkDevice device{};
        VkSurfaceKHR surface{};

        vk_gpu_info gpu;

#pragma region Queues
        uint32_t graphics_family_index{};
        VkQueue graphics_queue{};
        uint32_t compute_family_index{};
        VkQueue compute_queue{};
        uint32_t transfer_family_index{};
        VkQueue copy_queue{};
#pragma endregion

        vulkan_render_engine(nova_settings& settings, nova::ttl::task_scheduler* task_scheduler, RENDERDOC_API_1_3_0* renderdoc);

        vulkan_render_engine(vulkan_render_engine&& other) = delete;
        vulkan_render_engine& operator=(vulkan_render_engine&& other) noexcept = delete;

        vulkan_render_engine(const vulkan_render_engine& other) = delete;
        vulkan_render_engine& operator=(const vulkan_render_engine& other) = delete;

        ~vulkan_render_engine() override;

        void flush_model_matrix_buffer();

        void render_frame() override;

        std::shared_ptr<iwindow> get_window() const override;

        void set_shaderpack(const shaderpack_data& data) override;

        result<renderable_id_t> add_renderable(const static_mesh_renderable_data& data) override;

        void set_renderable_visibility(renderable_id_t id, bool is_visible) override;

        void delete_renderable(renderable_id_t id) override;

        result<mesh_id_t> add_mesh(const mesh_data& input_mesh) override;

        void delete_mesh(uint32_t mesh_id) override;

        /*!
         * \brief Retrieves the command pool for the current thread, or creates a new one if there is nothing or the
         * current thread
         *
         * \param queue_index the index of the queue we need to get a command pool for
         *
         * \return The command pool for the current thread
         */
        VkCommandPool get_command_buffer_pool_for_current_thread(uint32_t queue_index);

    private:
        /*!
         * \brief The number of frames that Nova can have in-flight at a given time
         *
         * The default value seen here is the number of in-flight frames we request from the GPU. However, the GPU may
         * create more swapchain images than we requested - which is fine, but that means that this value at runtime
         * may not be what it is here. Be aware.
         *
         * This value should not be changed outside of the constructor, but I don't know how to make the compiler
         * enforce that - so be careful!
         */
        uint32_t max_in_flight_frames = 3;
        uint32_t current_swapchain_image = 0;
        uint32_t current_frame = 0;

        std::vector<const char*> enabled_layer_names;

#ifdef NOVA_LINUX
        std::shared_ptr<x11_window> window;
#elif defined(NOVA_WINDOWS)
        std::shared_ptr<win32_window> window;
#endif

#pragma region Globals
        RENDERDOC_API_1_3_0* renderdoc;

        VkInstance vk_instance{};

        VmaAllocator vma_allocator{};

        std::vector<VkSemaphore> render_finished_semaphores;
        std::vector<VkSemaphore> image_available_semaphores;

        /*!
         * \brief Fences to tell us if we can render the next frame
         */
        std::vector<VkFence> frame_fences;

        bool dynamic_textures_need_to_transition = false;
        std::unordered_map<std::string, vk_texture> textures;
        std::unordered_map<std::string, vk_buffer> buffers;

        VkSampler point_sampler{};

        /*!
         * \brief Thread-local command pools so multiple tasks don't try to use the same command pools at the same time
         */
        std::vector<std::unordered_map<uint32_t, VkCommandPool>> command_pools_by_thread_idx;

        std::vector<VkDescriptorPool> descriptor_pools_by_thread_idx;

        void reset_render_finished_semaphores();

        /*!
         * \brief Fills out the `command_pools_by_thread_idx` member
         */
        void create_per_thread_command_pools();

        /*!
         * \brief Fills out the `descriptor_pools_by_thread_idx` member
         */
        void create_per_thread_descriptor_pools();

        /*!
         * \brief Allocates new command pools - one for graphics, one for transfer, one for compute
         */
        std::unordered_map<uint32_t, VkCommandPool> make_new_command_pools() const;

        /*!
         * \brief Factory function to make a new descriptor pool
         */
        VkDescriptorPool make_new_descriptor_pool() const;

        /*!
         * \brief Retrieves the descriptor pool for the calling thread
         */
        VkDescriptorPool get_descriptor_pool_for_current_thread();

        /*!
         * \brief Creates a point sampler and a linear sampler
         */
        void create_default_samplers();
#pragma endregion

#pragma region Init
    protected:
        void open_window(uint32_t width, uint32_t height) override;

    private:
        static bool does_device_support_extensions(VkPhysicalDevice device);

        void create_device();
        void create_memory_allocator();

        void create_global_sync_objects();
#pragma endregion

#pragma region Swapchain
        std::unique_ptr<swapchain_manager> swapchain;

        void create_swapchain();
#pragma endregion

#pragma region Shaderpack
        bool shaderpack_loaded = false;
        std::mutex shaderpack_loading_mutex;
        shaderpack_data shaderpack;

        std::unordered_map<std::string, vk_render_pass> render_passes;
        std::vector<std::string> render_passes_by_order;

        std::unordered_map<std::string, material_data> materials;

        /*!
         * \brief Gets all the descriptor bindings from the provided SPIR-V code, performing basic validation that the
         * user hasn't declared two different bindings with the same name
         *
         * \param spirv The SPIR-V shader code to get bindings from
         * \param shader_stage The shader stage the binding was found in
         * \param bindings An in/out array that holds all the existing binding before this method, and holds the
         * existing bindings plus new ones declared in the shader after this method
         */
        static void get_shader_module_descriptors(const std::vector<uint32_t>& spirv,
                                                  VkShaderStageFlags shader_stage,
                                                  std::unordered_map<std::string, vk_resource_binding>& bindings);

        /*!
         * \brief Adds information about the provided resource to `bindings`
         *
         * If there's nothing at the descriptor set and binding of `resource`, `resource` is added directly to `bindings`.
         * If something already exists, `bindings` is unchanged
         *
         * \param bindings The map of bindings to ass data to
         * \param shader_stage The shader stage the binding was found in
         * \param shader_compiler The compiler used for the shader that the resource came from
         * \param resource The resource to maybe add to `bindings
         * \param type The type of this resource
         */
        static void add_resource_to_bindings(std::unordered_map<std::string, vk_resource_binding>& bindings,
                                             VkShaderStageFlags shader_stage,
                                             const spirv_cross::CompilerGLSL& shader_compiler,
                                             const spirv_cross::Resource& resource,
                                             VkDescriptorType type);

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
         * \param all_bindings All the bindings we know about. This is expected to cover sets 0 - n, with all whole
         * numbers between 0 and n represented
         * \return A list of descriptor set layouts, one for each set in `bindings`
         */
        std::vector<VkDescriptorSetLayout> create_descriptor_set_layouts(
            const std::unordered_map<std::string, vk_resource_binding>& all_bindings) const;

        /*!
         * \brief Creates descriptor sets for all the materials that are loaded
         */
        void create_material_descriptor_sets();

        std::vector<VkImageMemoryBarrier> make_attachment_to_shader_read_only_barriers(const std::unordered_set<std::string>& textures);

        void create_barriers_for_renderpass(vk_render_pass& pass);

        /*!
         * \brief Looks at all the renderpasses and generates barriers for resources that are written to in one pass
         * and read from in a downstream pass
         */
        void generate_barriers_for_dynamic_resources();

        /*!
         * \brief Binds this material's resources to its descriptor sets
         *
         * Prerequisite: This function must be run after create_material_descriptor_sets
         */
        void update_material_descriptor_sets(const material_pass& mat,
                                             const std::unordered_map<std::string, vk_resource_binding>& name_to_descriptor);

        /*!
         * \brier Helper function so I don't have the same code for both dynamic and builtin textures
         *
         * \param texture The texture to write to the descriptor set
         * \param write a VkWriteDescriptorSet struct that we can add information about out descriptor to
         * \param image_infos A place to store VkDescriptorImageInfo structs so they don't get cleaned up too early
         */
        void write_texture_to_descriptor(const vk_texture& texture,
                                         VkWriteDescriptorSet& write,
                                         std::vector<VkDescriptorImageInfo>& image_infos) const;

        /*!
         * \brief Helper function so I don't have the same code for dynamic and builtin buffers
         *
         * \param buffer The buffer to write to the descriptor set
         * \param write A VkWriteDescriptorSet struct that we can add information about our descriptor to
         * \param buffer_infos A place to store VkDescriptorBufferInfo structs so they don't get cleaned up too early
         * \param type The type of buffer that the descriptor describes
         */
        static void write_buffer_to_descriptor(const VkBuffer& buffer,
                                               VkWriteDescriptorSet& write,
                                               std::vector<VkDescriptorBufferInfo>& buffer_infos,
                                               VkDescriptorType type);

        /*!
         * \brief Executed barriers for all the dynamic textures so they are in COLOR_ATTACHMENT_OPTIMAL layout
         */
        void transition_dynamic_textures();

        /*!
         * \brief Converts the list of attachment names into attachment descriptions and references that can be later
         * used to make a VkRenderpass
         *
         * \param attachment_names The names of the attachments we want to convert
         * \return VkAttachmentDescriptions and VkAttachmentReferences for the attachments we want to convert
         */
        std::pair<std::vector<VkAttachmentDescription>, std::vector<VkAttachmentReference>> to_vk_attachment_info(
            std::vector<std::string>& attachment_names);

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
        void destroy_dynamic_resources();
#pragma endregion

#pragma region Mesh
        // Might need to make 64-bit keys eventually, but in 2018 it's not a concern
        std::unordered_map<uint32_t, vk_mesh> meshes;
        std::mutex meshes_mutex;
        std::atomic<uint32_t> next_mesh_id = 0;

        /*!
         * \brief Validates that the sizes in `options` are properly aligned
         *
         * `options.buffer_part_size` must be a whole-number multiple of `sizeof(full_vertex)`
         * `options.new_buffer_size` must be a whole-number multiple of `options.buffer_part_size`
         * `options.max_total_allocation` must be a whole-number multiple of `options.new_buffer_size`
         */
        void validate_mesh_options(const nova_settings::block_allocator_settings& options) const;
#pragma endregion

#pragma region Renderables
        /*!
         * \brief All the renderables that Nova will process
         */
        std::unordered_map<renderable_id_t, renderable_metadata> metadata_for_renderables;

        result<std::vector<const material_pass*>> get_material_passes_for_renderable(const static_mesh_renderable_data& data);

        result<const vk_mesh*> get_mesh_for_renderable(const static_mesh_renderable_data& data);

        result<renderable_id_t> register_renderable(const static_mesh_renderable_data& data,
                                                    const vk_mesh* mesh,
                                                    const std::vector<const material_pass*>& passes);
#pragma endregion

#pragma region Rendering
        /*!
         * \brief A buffer to hold model matrices for all render objects
         *
         * Nova puts all the model matrices for all the objects into a single buffer, then indexes into that from
         * shaders. This allows Nova to make heavy use of instanced rendering
         */
        vk_buffer model_matrix_buffer;

        /*!
         * \brief The data that's constant for the whole frame
         *
         * Includes things like the world time
         */
        vk_buffer per_frame_data_buffer;

        std::unordered_map<std::string, std::vector<vk_pipeline>> pipelines_by_renderpass;
        std::unordered_map<std::string, std::vector<material_pass>> material_passes_by_pipeline;
        std::unordered_map<std::string, vk_renderables> renderables_by_material;

        std::mutex rendering_mutex;
        std::condition_variable rendering_cv;

        /*!
         * \brief The index in the model matrix buffer for write a model matrix to
         *
         * When Nova generates drawcalls, it writes model matrices to a single buffer. This variable keeps track of the
         * current write position
         *
         * This will need to be reworked a lot for multithreaded rendering. The current plan for multithreaded
         * rendering is to make each material's drawcalls a separate async task. This means that each task will have to
         * be given a separate piece of the model matrix buffer. This variable will still be used at a high level, but
         * each task will need a local index into the model matrix buffer
         */
        uint32_t cur_model_matrix_idx = 0;

        void create_builtin_uniform_buffers();

        /*!
         * \brief Performs all tasks necessary to render this renderpass
         *
         * This method fill start a separate async task for each pipeline that is in the given renderpass
         *
         * \param renderpass_name The name of the renderpass to execute
         * \param cmds The command buffer to record this renderpass into
         */
        void record_renderpass(const std::string* renderpass_name, VkCommandBuffer cmds);

        /*!
         * \brief Renders all the meshes that use a single pipeline
         *
         * This method does not start any async tasks
         *
         * This method does allocate a secondary command buffer, which is returned through the `cmds` out parameter.
         * Intended use case is to render the things for each pipeline in a separate fiber, but imma have to do a lot
         * of profiling to be sure
         */
        void record_pipeline(const vk_pipeline* pipeline, VkCommandBuffer* cmds, const vk_render_pass& renderpass);

        /*!
         * \brief Binds all the resources that the provided material uses to the given pipeline
         *
         * \param pass The material pass to get resources from
         * \param pipeline The pipeline to get binding locations from
         * \param cmds The command buffer to bind things in
         */
        void bind_material_resources(const material_pass& pass, const vk_pipeline& pipeline, VkCommandBuffer cmds);

        /*!
         * \brief Renders all the things using the provided material
         */
        void record_drawing_all_for_material(const material_pass& pass, VkCommandBuffer cmds);

        /*!
         * \brief Submits the provided command buffer to the provided queue
         *
         * This method is thread-safe
         *
         * \param cmds The command buffer to submit
         * \param queue The queue to submit the command buffer to
         * \param cmd_buffer_done_fence The fence to signal when the command buffer has finished executing
         * \param wait_semaphores Any semaphores that the command buffer needs to wait on
         * \param signal_semaphores The semaphores to signal when the command buffer is done
         *
         * \pre cmds is a fully recorded command buffer
         */
        void submit_to_queue(VkCommandBuffer cmds,
                             VkQueue queue,
                             VkFence cmd_buffer_done_fence = {},
                             const std::vector<VkSemaphore>& wait_semaphores = {},
                             const std::vector<VkSemaphore>& signal_semaphores = {});
#pragma endregion

        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;

        VkDebugUtilsMessengerEXT debug_callback{};

        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;

        static VkFormat to_vk_format(pixel_format_enum format);
    };

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                         VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                         void* pUserData);
} // namespace nova::renderer

#endif // NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
