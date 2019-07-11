/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#pragma once
#include "gl3_command_list.hpp"
#include "glad/glad.h"
#include "nova_renderer/render_engine.hpp"
#include <EASTL/shared_ptr.h>

namespace nova::renderer::rhi {
    /*!
     * \brief OpenGL 3.1 render engine because compatibility
     */
    class Gl3RenderEngine final : public RenderEngine {
    public:
        explicit Gl3RenderEngine(NovaSettings& settings);

        Gl3RenderEngine(Gl3RenderEngine&& other) = delete;
        Gl3RenderEngine& operator=(Gl3RenderEngine&& other) noexcept = delete;

        Gl3RenderEngine(const Gl3RenderEngine& other) = delete;
        Gl3RenderEngine& operator=(const Gl3RenderEngine& other) = delete;

        ~Gl3RenderEngine() override final = default;

        eastl::shared_ptr<Window> get_window() const override final;

        void set_num_renderpasses(uint32_t num_renderpasses) override final;

        Result<DeviceMemory*> allocate_device_memory(uint64_t size, MemoryUsage type, ObjectType allowed_objects) override final;

        // Inherited via render_engine
        Result<Renderpass*> create_renderpass(const shaderpack::RenderPassCreateInfo& data) override final;

        Framebuffer* create_framebuffer(const Renderpass* renderpass,
                                        const eastl::vector<Image*>& attachments,
                                        const glm::uvec2& framebuffer_size) override final;

        DescriptorPool* create_descriptor_pool(uint32_t num_sampled_images,
                                               uint32_t num_samplers,
                                               uint32_t num_uniform_buffers) override final;

        eastl::vector<DescriptorSet*> create_descriptor_sets(const PipelineInterface* pipeline_interface,
                                                           DescriptorPool* pool) override final;

        void update_descriptor_sets(eastl::vector<DescriptorSetWrite>& writes) override final;

        Result<PipelineInterface*> create_pipeline_interface(
            const eastl::unordered_map<eastl::string, ResourceBindingDescription>& bindings,
            const eastl::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
            const eastl::optional<shaderpack::TextureAttachmentInfo>& depth_texture) override final;

        Result<Pipeline*> create_pipeline(PipelineInterface* pipeline_interface, const shaderpack::PipelineCreateInfo& data) override final;

        Buffer* create_buffer(const BufferCreateInfo& info) override final;

        /*!
         * \inheritdoc
         *
         * This method assumes you're going to use the buffer as a copy source
         */
        void write_data_to_buffer(const void* data, uint64_t num_bytes, uint64_t offset, const Buffer* buffer) override final;

        Image* create_texture(const shaderpack::TextureCreateInfo& info) override final;

        Semaphore* create_semaphore() override final;
        eastl::vector<Semaphore*> create_semaphores(uint32_t num_semaphores) override final;

        Fence* create_fence(bool signaled = false) override final;

        eastl::vector<Fence*> create_fences(uint32_t num_fences, bool signaled = false) override final;

        void wait_for_fences(const eastl::vector<Fence*> fences) override final;

        void reset_fences(const eastl::vector<Fence*>& fences) override final;

        void destroy_renderpass(Renderpass* pass) override final;

        void destroy_framebuffer(Framebuffer* framebuffer) override final;

        void destroy_pipeline_interface(PipelineInterface* pipeline_interface) override final;

        void destroy_pipeline(Pipeline* pipeline) override final;
        void destroy_texture(Image* resource) override final;
        void destroy_semaphores(eastl::vector<Semaphore*>& semaphores) override final;
        void destroy_fences(eastl::vector<Fence*>& fences) override final;

        CommandList* get_command_list(uint32_t thread_idx,
                                      QueueType needed_queue_type,
                                      CommandList::Level command_list_type) override final;
        
        void submit_command_list(CommandList* cmds,
                                 QueueType queue,
                                 Fence* fence_to_signal = nullptr,
                                 const eastl::vector<Semaphore*>& wait_semaphores = {},
                                 const eastl::vector<Semaphore*>& signal_semaphores = {}) override final;

    protected:
        void open_window_and_create_surface(const NovaSettings::WindowOptions& options);

    private:
        bool supports_geometry_shaders = false;

        eastl::unordered_map<eastl::string, shaderpack::SamplerCreateInfo> samplers;

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

    Result<GLuint> compile_shader(const eastl::vector<uint32_t>& spirv, GLenum shader_type);
} // namespace nova::renderer::rhi
