#pragma once

#include "nova_renderer/rhi/render_engine.hpp"
#include "nova_renderer/window.hpp"

// BE CAREFUL WHERE WE INCLUDE GLAD
#include "gl3_command_list.hpp"
#include "glad/glad.h"

namespace nova::renderer::rhi {
    struct NvGlDeviceInfo {
        mem::Bytes max_uniform_buffer_size = 0;
    };

    /*!
     * \brief OpenGL 4.5 RHI backend. Optimized for Nvidia GPUs
     */
    class Gl4NvRenderEngine final : public RenderEngine {
        // Some optimizations that still need to happen:
        // - Compile shaders to ARB assembly in 1500 ms or less
        // - Only one VAO
    public:
        Gl4NvRenderEngine(NovaSettingsAccessManager& settings,
                          const std::shared_ptr<NovaWindow>& window,
                          mem::AllocatorHandle<>& allocator);

        Gl4NvRenderEngine(Gl4NvRenderEngine&& other) = delete;
        Gl4NvRenderEngine& operator=(Gl4NvRenderEngine&& other) noexcept = delete;

        Gl4NvRenderEngine(const Gl4NvRenderEngine& other) = delete;
        Gl4NvRenderEngine& operator=(const Gl4NvRenderEngine& other) = delete;

        ~Gl4NvRenderEngine() override;

        void set_num_renderpasses(uint32_t num_renderpasses) override;

        ntl::Result<DeviceMemory*> allocate_device_memory(mem::Bytes size,
                                                          MemoryUsage type,
                                                          ObjectType allowed_objects,
                                                          mem::AllocatorHandle<>& allocator) override;

        // Inherited via render_engine
        ntl::Result<Renderpass*> create_renderpass(const shaderpack::RenderPassCreateInfo& data,
                                                   const glm::uvec2& framebuffer_size,
                                                   mem::AllocatorHandle<>& allocator) override;

        Framebuffer* create_framebuffer(const Renderpass* renderpass,
                                        const std::pmr::vector<Image*>& color_attachments,
                                        const std::optional<Image*> depth_attachment,
                                        const glm::uvec2& framebuffer_size,
                                        mem::AllocatorHandle<>& allocator) override;

        DescriptorPool* create_descriptor_pool(uint32_t num_sampled_images,
                                               uint32_t num_samplers,
                                               uint32_t num_uniform_buffers,
                                               mem::AllocatorHandle<>& allocator) override;

        std::pmr::vector<DescriptorSet*> create_descriptor_sets(const PipelineInterface* pipeline_interface,
                                                                DescriptorPool* pool,
                                                                mem::AllocatorHandle<>& allocator) override;

        void update_descriptor_sets(std::pmr::vector<DescriptorSetWrite>& writes) override;

        ntl::Result<PipelineInterface*> create_pipeline_interface(
            const std::unordered_map<std::string, ResourceBindingDescription>& bindings,
            const std::pmr::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
            const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture,
            mem::AllocatorHandle<>& allocator) override;

        ntl::Result<Pipeline*> create_pipeline(PipelineInterface* pipeline_interface,
                                               const shaderpack::PipelineCreateInfo& data,
                                               mem::AllocatorHandle<>& allocator) override;

        Buffer* create_buffer(const BufferCreateInfo& info, DeviceMemoryResource& memory, mem::AllocatorHandle<>& allocator) override;

        /*!
         * \inheritdoc
         *
         * This method assumes you're going to use the buffer as a copy source
         */
        void write_data_to_buffer(const void* data, mem::Bytes num_bytes, mem::Bytes offset, const Buffer* buffer) override;

        Image* create_image(const shaderpack::TextureCreateInfo& info, mem::AllocatorHandle<>& allocator) override;

        Semaphore* create_semaphore(mem::AllocatorHandle<>& allocator) override;
        std::pmr::vector<Semaphore*> create_semaphores(uint32_t num_semaphores, mem::AllocatorHandle<>& allocator) override;

        Fence* create_fence(mem::AllocatorHandle<>& allocator, bool signaled = false) override;

        std::pmr::vector<Fence*> create_fences(mem::AllocatorHandle<>& allocator, uint32_t num_fences, bool signaled = false) override;

        void wait_for_fences(std::pmr::vector<Fence*> fences) override;

        void reset_fences(const std::pmr::vector<Fence*>& fences) override;

        void destroy_renderpass(Renderpass* pass, mem::AllocatorHandle<>& allocator) override;

        void destroy_framebuffer(Framebuffer* framebuffer, mem::AllocatorHandle<>& allocator) override;

        void destroy_pipeline_interface(PipelineInterface* pipeline_interface, mem::AllocatorHandle<>& allocator) override;

        void destroy_pipeline(Pipeline* pipeline, mem::AllocatorHandle<>& allocator) override;

        void destroy_texture(Image* resource, mem::AllocatorHandle<>& allocator) override;

        void destroy_semaphores(std::pmr::vector<Semaphore*>& semaphores, mem::AllocatorHandle<>& allocator) override;

        void destroy_fences(const std::pmr::vector<Fence*>& fences, mem::AllocatorHandle<>& allocator) override;

        CommandList* create_command_list(mem::AllocatorHandle<>& allocator,
                                         uint32_t thread_idx,
                                         QueueType needed_queue_type,
                                         CommandList::Level command_list_type) override;

        void submit_command_list(CommandList* cmds,
                                 QueueType queue,
                                 Fence* fence_to_signal = nullptr,
                                 const std::pmr::vector<Semaphore*>& wait_semaphores = {},
                                 const std::pmr::vector<Semaphore*>& signal_semaphores = {}) override;

    private:
        NvGlDeviceInfo gl_info;

        bool supports_geometry_shaders = false;

        std::unordered_map<std::string, shaderpack::SamplerCreateInfo> samplers;

        mem::AllocatorHandle<Gl3CommandList>* command_allocator;

        void save_device_info();

        static void set_initial_state();

#pragma region Command list execution
        static void copy_buffers_impl(const Gl3BufferCopyCommand& buffer_copy);

        static void begin_renderpass_impl(const Gl3BeginRenderpassCommand& begin_renderpass);

        static void bind_pipeline_impl(const Gl3BindPipelineCommand& bind_pipeline);

        static void bind_descriptor_sets_impl(const Gl3BindDescriptorSetsCommand& bind_descriptor_sets);

        static void bind_vertex_buffers_impl(const Gl3BindVertexBuffersCommand& bind_vertex_buffers);

        static void bind_index_buffer_impl(const Gl3BindIndexBufferCommand& bind_index_buffer);

        static void draw_indexed_mesh_impl(const Gl3DrawIndexedMeshCommand& draw_indexed_mesh);

        static void execute_command_lists_impl(const Gl3ExecuteCommandListsCommand& execute_command_lists);
#pragma endregion
    };

    ntl::Result<GLuint> compile_shader(const std::pmr::vector<uint32_t>& spirv, GLenum shader_type);
} // namespace nova::renderer::rhi
