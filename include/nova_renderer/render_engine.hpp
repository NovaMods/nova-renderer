//
// Created by jannis on 29.08.18.
//

#ifndef NOVA_RENDERER_RENDER_ENGINE_HPP
#define NOVA_RENDERER_RENDER_ENGINE_HPP

#include <memory>

#include "../../src/windowing/win32_window.hpp"
#include "nova_renderer/nova_settings.hpp"
#include "nova_renderer/shaderpack_data.hpp"
#include "nova_renderer/util/platform.hpp"
#include "nova_renderer/util/result.hpp"
#include "nova_renderer/util/utils.hpp"
#include "nova_renderer/window.hpp"
#include "nova_renderer/rhi_types.hpp"
#include "nova_renderer/command_list.hpp"

namespace nova::renderer::rhi {
    NOVA_EXCEPTION(render_engine_initialization_exception);
    NOVA_EXCEPTION(render_engine_rendering_exception);

#define NUM_THREADS 1

    /*!
     * \brief Abstract class for render backends
     *
     * The constructor should not make any initialization
     * All functions must be called after init(nova::settings) has been called except
     *   explicitly marked in the documentation
     */
    class RenderEngine {
    public:
        RenderEngine(RenderEngine&& other) = delete;
        RenderEngine& operator=(RenderEngine&& other) noexcept = delete;

        RenderEngine(const RenderEngine& other) = delete;
        RenderEngine& operator=(const RenderEngine& other) = delete;

        /*!
         * \brief Needed to make destructor of subclasses called
         */
        virtual ~RenderEngine() = default;

        [[nodiscard]] virtual std::shared_ptr<Window> get_window() const = 0;

        /*!
         * \brief Allows the user to set an allocator that will be used for per-shaderpack objects. This allocator will
         * be cleaned up wen a new shaderpack is loaded, so I don't need to worry about cleaning up my memory before
         * that
         */
        virtual void set_shaderpack_data_allocator() = 0;

        virtual void set_num_renderpasses(uint32_t num_renderpasses) = 0;

        [[nodiscard]] virtual Result<DeviceMemory*> allocate_device_memory(uint64_t size, MemoryUsage type, ObjectType allowed_objects) = 0;

        /*!
         * \brief Creates a renderpass from the provided data
         *
         * Renderpasses are created 100% upfront, meaning that the caller can't change anything about a renderpass
         * after it's been created
         *
         * \param data The data to create a renderpass from
         *
         * \return The newly created renderpass
         */
        [[nodiscard]] virtual Result<Renderpass*> create_renderpass(const shaderpack::RenderPassCreateInfo& data) = 0;

        [[nodiscard]] virtual Framebuffer* create_framebuffer(const Renderpass* renderpass,
                                                              const std::vector<Image*>& attachments,
                                                              const glm::uvec2& framebuffer_size) = 0;

        [[nodiscard]] virtual Result<PipelineInterface*> create_pipeline_interface(
            const std::unordered_map<std::string, ResourceBindingDescription>& bindings,
            const std::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
            const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) = 0;

        [[nodiscard]] virtual DescriptorPool* create_descriptor_pool(uint32_t num_sampled_images,
                                                                     uint32_t num_samplers,
                                                                     uint32_t num_uniform_buffers) = 0;

        [[nodiscard]] virtual std::vector<DescriptorSet*> create_descriptor_sets(const PipelineInterface* pipeline_interface,
                                                                                 const DescriptorPool* pool) = 0;

        virtual void update_descriptor_sets(std::vector<DescriptorSetWrite>& writes) = 0;

        [[nodiscard]] virtual Result<Pipeline*> create_pipeline(const PipelineInterface* pipeline_interface,
                                                                const shaderpack::PipelineCreateInfo& data) = 0;

        [[nodiscard]] virtual Buffer* create_buffer(const BufferCreateInfo& info) = 0;

        [[nodiscard]] virtual Image* create_texture(const shaderpack::TextureCreateInfo& info) = 0;

        [[nodiscard]] virtual Semaphore* create_semaphore() = 0;

        [[nodiscard]] virtual std::vector<Semaphore*> create_semaphores(uint32_t num_semaphores) = 0;

        [[nodiscard]] virtual Fence* create_fence(bool signaled = false) = 0;

        [[nodiscard]] virtual std::vector<Fence*> create_fences(uint32_t num_fences, bool signaled = false) = 0;

        virtual void destroy_renderpass(Renderpass* pass) = 0;

        virtual void destroy_framebuffer(const Framebuffer* framebuffer) = 0;

        virtual void destroy_pipeline_interface(const PipelineInterface* pipeline_interface) = 0;

        virtual void destroy_pipeline(Pipeline* pipeline) = 0;

        virtual void destroy_texture(Image* resource) = 0;

        virtual void destroy_semaphores(const std::vector<Semaphore*>& semaphores) = 0;

        virtual void destroy_fences(const std::vector<Fence*>& fences) = 0;

        /*!
         * \brief Allocates a new command list that can be used from the provided thread and has the desired type
         *
         * Ownership of the command list is given to the caller. You can record your commands into it, then submit it
         * to a queue. Submitting it gives ownership back to the render engine, and recording commands into a
         * submitted command list is not supported
         *
         * There is one command list pool per swapchain image per thread. All the pools for one swapchain image are
         * reset at the beginning of a frame that renders to that swapchain image. This means that any command list
         * allocated in one frame will not be valid in the next frame. DO NOT hold on to command lists
         *
         * Command lists allocated by this method are returned ready to record commands into - the caller doesn't need
         * to begin the command list
         */
        virtual CommandList* allocate_command_list(uint32_t thread_idx, QueueType needed_queue_type, CommandList::Level level) = 0;

        virtual void submit_command_list(CommandList* cmds,
                                         QueueType queue,
                                         Fence* fence_to_signal = nullptr,
                                         const std::vector<Semaphore*>& wait_semaphores = {},
                                         const std::vector<Semaphore*>& signal_semaphores = {}) = 0;

    protected:
        NovaSettings& settings;

#ifdef NOVA_LINUX
        std::shared_ptr<x11_window> window;
#elif defined(NOVA_WINDOWS)
        std::shared_ptr<win32_window> window;
#endif

        glm::uvec2 swapchain_size;

        /*!
         * \brief Initializes the engine, does **NOT** open any window
         * \param settings The settings passed to nova
         *
         * Intentionally does nothing. This constructor serves mostly to ensure that concrete render engines have a
         * constructor that takes in some settings
         *
         * \attention Called by nova
         */
        explicit RenderEngine(NovaSettings& settings) : settings(settings), swapchain_size(settings.window.width, settings.window.height){};

        /*!
         * \brief Initializes the window with the given size, and creates the swapchain for that window
         * \param options The window options. Includes the size and title of the windows
         */
        virtual void open_window_and_create_surface(const NovaSettings::WindowOptions& options) = 0;
    };
} // namespace nova::renderer::rhi

#endif // NOVA_RENDERER_RENDER_ENGINE_HPP
