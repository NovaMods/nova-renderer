#pragma once

#include <memory>

#include "nova_renderer/command_list.hpp"
#include "nova_renderer/nova_settings.hpp"
#include "nova_renderer/rhi_types.hpp"
#include "nova_renderer/shaderpack_data.hpp"
#include "nova_renderer/util/result.hpp"
#include "nova_renderer/window.hpp"

namespace nova::renderer::rhi {
    struct Fence;
    struct Image;
    struct Semaphore;

    class Swapchain;

    /*!
     * \brief All the GPU architectures that Nova cares about, at whatever granularity is most useful
     */
    enum class DeviceArchitecture {
        Unknown,

        Amd,
        Nvidia,
        Intel,
    };

    /*!
     * \brief Information about hte capabilities and limits of the device we're running on
     */
    struct DeviceInfo {
        DeviceArchitecture architecture = DeviceArchitecture::Unknown;

        uint64_t max_uniform_buffer_size = 0;
        uint64_t max_texture_size = 0;

        uint64_t total_device_memory = 0;
        bool is_uma = false;

        bool supports_raytracing = false;
        bool supports_mesh_shaders = false;
    };

#define NUM_THREADS 1

    /*!
     * \brief Interface to a logical device which can render to an operating system window
     */
    class RenderEngine {
    public:
        DeviceInfo info;

        NovaSettingsAccessManager& settings;

        RenderEngine(RenderEngine&& other) = delete;
        RenderEngine& operator=(RenderEngine&& other) noexcept = delete;

        RenderEngine(const RenderEngine& other) = delete;
        RenderEngine& operator=(const RenderEngine& other) = delete;

        /*!
         * \brief Needed to make destructor of subclasses called
         */
        virtual ~RenderEngine() = default;

        [[nodiscard]] Window& get_window() const;

        /*!
         * \brief Allows the user to set an allocator that will be used for per-shaderpack objects. This allocator will
         * be cleaned up wen a new shaderpack is loaded, so I don't need to worry about cleaning up my memory before
         * that
         */
        void set_shaderpack_data_allocator(const bvestl::polyalloc::allocator_handle& allocator_handle);

        virtual void set_num_renderpasses(uint32_t num_renderpasses) = 0;

        [[nodiscard]] virtual ntl::Result<DeviceMemory*> allocate_device_memory(uint64_t size,
                                                                                MemoryUsage type,
                                                                                ObjectType allowed_objects) = 0;

        /*!
         * \brief Creates a renderpass from the provided data
         *
         * Renderpasses are created 100% upfront, meaning that the caller can't change anything about a renderpass
         * after it's been created
         *
         * \param data The data to create a renderpass from
         * \param framebuffer_size The size in pixels of the framebuffer that the renderpass will write to
         *
         * \return The newly created renderpass
         */
        [[nodiscard]] virtual ntl::Result<Renderpass*> create_renderpass(const shaderpack::RenderPassCreateInfo& data,
                                                                         const glm::uvec2& framebuffer_size) = 0;

        [[nodiscard]] virtual Framebuffer* create_framebuffer(const Renderpass* renderpass,
                                                              const std::vector<Image*>& color_attachments,
                                                              const std::optional<Image*> depth_attachment,
                                                              const glm::uvec2& framebuffer_size) = 0;

        [[nodiscard]] virtual ntl::Result<PipelineInterface*> create_pipeline_interface(
            const std::unordered_map<std::string, ResourceBindingDescription>& bindings,
            const std::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
            const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) = 0;

        [[nodiscard]] virtual DescriptorPool* create_descriptor_pool(uint32_t num_sampled_images,
                                                                     uint32_t num_samplers,
                                                                     uint32_t num_uniform_buffers) = 0;

        [[nodiscard]] virtual std::vector<DescriptorSet*> create_descriptor_sets(const PipelineInterface* pipeline_interface,
                                                                                 DescriptorPool* pool) = 0;

        virtual void update_descriptor_sets(std::vector<DescriptorSetWrite>& writes) = 0;

        [[nodiscard]] virtual ntl::Result<Pipeline*> create_pipeline(PipelineInterface* pipeline_interface,
                                                                     const shaderpack::PipelineCreateInfo& data) = 0;

        /*!
         * \brief Creates a buffer with undefined contents
         */
        [[nodiscard]] virtual Buffer* create_buffer(const BufferCreateInfo& info, DeviceMemoryResource& memory) = 0;

        /*!
         * \brief Writes data to a buffer
         *
         * This method always writes the data from byte 0 to byte num_bytes. It does not let you use an offset for either reading from
         * the data or writing to the buffer
         *
         * The CPU must be able to write directly to the buffer for this method to work. If the buffer is device local, this method will
         * fail in a horrible way
         *
         * \param data The data to upload
         * \param num_bytes The number of bytes to write
         * \param offset The offset from the start of the buffer to write the data at
         * \param buffer The buffer to write to
         */
        virtual void write_data_to_buffer(const void* data, uint64_t num_bytes, uint64_t offset, const Buffer* buffer) = 0;

        [[nodiscard]] virtual Image* create_image(const shaderpack::TextureCreateInfo& info) = 0;

        [[nodiscard]] virtual Semaphore* create_semaphore() = 0;

        [[nodiscard]] virtual std::vector<Semaphore*> create_semaphores(uint32_t num_semaphores) = 0;

        [[nodiscard]] virtual Fence* create_fence(bool signaled = false) = 0;

        [[nodiscard]] virtual std::vector<Fence*> create_fences(uint32_t num_fences, bool signaled = false) = 0;

        /*!
         * \blocks the fence until all fences are signaled
         *
         * Fences are waited on for an infinite time
         *
         * \param fences All the fences to wait for
         */
        virtual void wait_for_fences(std::vector<Fence*> fences) = 0;

        virtual void reset_fences(const std::vector<Fence*>& fences) = 0;

        /*!
         * \brief Clean up any GPU objects a Renderpass may own
         *
         * While Renderpasses are per-shaderpack objects, and their CPU memory will be cleaned up when a new shaderpack is loaded, we still
         * need to clean up their GPU objects
         */
        virtual void destroy_renderpass(Renderpass* pass) = 0;

        /*!
         * \brief Clean up any GPU objects a Framebuffer may own
         *
         * While Framebuffers are per-shaderpack objects, and their CPU memory will be cleaned up when a new shaderpack is loaded, we still
         * need to clean up their GPU objects
         */
        virtual void destroy_framebuffer(Framebuffer* framebuffer) = 0;

        /*!
         * \brief Clean up any GPU objects a PipelineInterface may own
         *
         * While PipelineInterfaces are per-shaderpack objects, and their CPU memory will be cleaned up when a new shaderpack is loaded, we
         * still need to clean up their GPU objects
         */
        virtual void destroy_pipeline_interface(PipelineInterface* pipeline_interface) = 0;

        /*!
         * \brief Clean up any GPU objects a Pipeline may own
         *
         * While Pipelines are per-shaderpack objects, and their CPU memory will be cleaned up when a new shaderpack is loaded, we still
         * need to clean up their GPU objects
         */
        virtual void destroy_pipeline(Pipeline* pipeline) = 0;

        /*!
         * \brief Clean up any GPU objects an Image may own
         *
         * While Images are per-shaderpack objects, and their CPU memory will be cleaned up when a new shaderpack is loaded, we still need
         * to clean up their GPU objects
         */
        virtual void destroy_texture(Image* resource) = 0;

        /*!
         * \brief Clean up any GPU objects a Semaphores may own
         *
         * While Semaphores are per-shaderpack objects, and their CPU memory will be cleaned up when a new shaderpack is loaded, we still
         * need to clean up their GPU objects
         */
        virtual void destroy_semaphores(std::vector<Semaphore*>& semaphores) = 0;

        /*!
         * \brief Clean up any GPU objects a Fence may own
         *
         * While Fence are per-shaderpack objects, and their CPU memory will be cleaned up when a new shaderpack is loaded, we still need to
         * clean up their GPU objects
         */
        virtual void destroy_fences(std::vector<Fence*>& fences) = 0;

        [[nodiscard]] Swapchain* get_swapchain() const;

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
        virtual CommandList* get_command_list(uint32_t thread_idx,
                                              QueueType needed_queue_type,
                                              CommandList::Level level = CommandList::Level::Primary) = 0;

        virtual void submit_command_list(CommandList* cmds,
                                         QueueType queue,
                                         Fence* fence_to_signal = nullptr,
                                         const std::vector<Semaphore*>& wait_semaphores = {},
                                         const std::vector<Semaphore*>& signal_semaphores = {}) = 0;

    protected:
        std::unique_ptr<Window> window;

        glm::uvec2 swapchain_size = {};
        Swapchain* swapchain = nullptr;

        bvestl::polyalloc::allocator_handle shaderpack_allocator;

        DeviceInfo capabilities;

        /*!
         * \brief Initializes the engine, does **NOT** open any window
         * \param allocator The allocator nova is using
         * \param settings The settings passed to nova
         *
         * Intentionally does nothing. This constructor serves mostly to ensure that concrete render engines have a
         * constructor that takes in some settings
         *
         * \attention Called by the various render engine implementations
         */
        explicit RenderEngine(bvestl::polyalloc::Allocator* allocator,
                              NovaSettingsAccessManager& settings) // NOLINT(cppcoreguidelines-pro-type-member-init)
            : settings(settings),
              swapchain_size(settings.settings.window.width, settings.settings.window.height),
              shaderpack_allocator(allocator){};

        template <typename AllocType>
        AllocType* new_object() {
            void* mem = shaderpack_allocator.allocate(sizeof(AllocType));
            return new(mem) AllocType;
        }

        template <typename AllocType, typename... ArgTypes>
        AllocType* new_object(ArgTypes&&... args) {
            void* mem = shaderpack_allocator.allocate(sizeof(AllocType));
            return new(mem) AllocType(std::forward<ArgTypes>(args)...);
        }
    };
} // namespace nova::renderer::rhi
