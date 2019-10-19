#pragma once

#include <memory>
#include "gl3_command_list.hpp"
#include "glad/glad.h"
#include "nova_renderer/render_engine.hpp"

namespace nova::renderer::rhi {
    /*!
     * \brief OpenGL 4.6 RHI backend. Optimized for Nvidia GPUs
     */
    class Gl4NvRenderEngine final : public RenderEngine {
    public:
        explicit Gl4NvRenderEngine(NovaSettingsAccessManager& settings);

        Gl4NvRenderEngine(Gl4NvRenderEngine&& other) = delete;
        Gl4NvRenderEngine& operator=(Gl4NvRenderEngine&& other) noexcept = delete;

        Gl4NvRenderEngine(const Gl4NvRenderEngine& other) = delete;
        Gl4NvRenderEngine& operator=(const Gl4NvRenderEngine& other) = delete;

        ~Gl4NvRenderEngine() override = default;

        void set_num_renderpasses(uint32_t num_renderpasses) override;

        ntl::Result<DeviceMemory*> allocate_device_memory(uint64_t size, MemoryUsage type, ObjectType allowed_objects) override;

        // Inherited via render_engine
        ntl::Result<Renderpass*> create_renderpass(const shaderpack::RenderPassCreateInfo& data) override;

        Framebuffer* create_framebuffer(const Renderpass* renderpass,
                                        const std::vector<Image*>& attachments,
                                        const glm::uvec2& framebuffer_size) override;

        DescriptorPool* create_descriptor_pool(uint32_t num_sampled_images, uint32_t num_samplers, uint32_t num_uniform_buffers) override;

        std::vector<DescriptorSet*> create_descriptor_sets(const PipelineInterface* pipeline_interface, DescriptorPool* pool) override;

        void update_descriptor_sets(std::vector<DescriptorSetWrite>& writes) override;

        ntl::Result<PipelineInterface*> create_pipeline_interface(
            const std::unordered_map<std::string, ResourceBindingDescription>& bindings,
            const std::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
            const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) override;

        ntl::Result<Pipeline*> create_pipeline(PipelineInterface* pipeline_interface, const shaderpack::PipelineCreateInfo& data) override;

        Buffer* create_buffer(const BufferCreateInfo& info) override;

        /*!
         * \inheritdoc
         *
         * This method assumes you're going to use the buffer as a copy source
         */
        void write_data_to_buffer(const void* data, uint64_t num_bytes, uint64_t offset, const Buffer* buffer) override;

        Image* create_texture(const shaderpack::TextureCreateInfo& info) override;

        Semaphore* create_semaphore() override;
        std::vector<Semaphore*> create_semaphores(uint32_t num_semaphores) override;

        Fence* create_fence(bool signaled = false) override;

        std::vector<Fence*> create_fences(uint32_t num_fences, bool signaled = false) override;

        void wait_for_fences(std::vector<Fence*> fences) override;

        void reset_fences(const std::vector<Fence*>& fences) override;

        void destroy_renderpass(Renderpass* pass) override;

        void destroy_framebuffer(Framebuffer* framebuffer) override;

        void destroy_pipeline_interface(PipelineInterface* pipeline_interface) override;

        void destroy_pipeline(Pipeline* pipeline) override;
        void destroy_texture(Image* resource) override;
        void destroy_semaphores(std::vector<Semaphore*>& semaphores) override;
        void destroy_fences(std::vector<Fence*>& fences) override;

        CommandList* get_command_list(uint32_t thread_idx, QueueType needed_queue_type, CommandList::Level command_list_type) override;

        void submit_command_list(CommandList* cmds,
                                 QueueType queue,
                                 Fence* fence_to_signal = nullptr,
                                 const std::vector<Semaphore*>& wait_semaphores = {},
                                 const std::vector<Semaphore*>& signal_semaphores = {}) override;

    protected:
        void open_window_and_create_surface(const NovaSettings::WindowOptions& options);

    private:
        bool supports_geometry_shaders = false;

        std::unordered_map<std::string, shaderpack::SamplerCreateInfo> samplers;

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

    ntl::Result<GLuint> compile_shader(const std::vector<uint32_t>& spirv, GLenum shader_type);
} // namespace nova::renderer::rhi
