/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#pragma once
#include "glad.h"
#include "nova_renderer/render_engine.hpp"

namespace nova::renderer::rhi {
    /*!
     * \brief OpenGL 2.1 render engine because compatibility
     */
    class Gl2RenderEngine final : public RenderEngine {
    public:
        explicit Gl2RenderEngine(NovaSettings& settings);

        Gl2RenderEngine(Gl2RenderEngine&& other) = delete;
        Gl2RenderEngine& operator=(Gl2RenderEngine&& other) noexcept = delete;

        Gl2RenderEngine(const Gl2RenderEngine& other) = delete;
        Gl2RenderEngine& operator=(const Gl2RenderEngine& other) = delete;

        ~Gl2RenderEngine() override final = default;

        std::shared_ptr<Window> get_window() const override final;

        void set_num_renderpasses(uint32_t num_renderpasses) override final;

        // Inherited via render_engine
        Result<Renderpass*> create_renderpass(const shaderpack::RenderPassCreateInfo& data) override final;
        Framebuffer* create_framebuffer(const Renderpass* renderpass,
                                        const std::vector<Image*>& attachments,
                                        const glm::uvec2& framebuffer_size) override final;
        Pipeline* create_pipeline(const Renderpass* renderpass, const shaderpack::PipelineCreateInfo& data) override final;
        Buffer* create_buffer(const BufferCreateInfo& info) override final;
        Image* create_texture(const shaderpack::TextureCreateInfo& info) override final;
        Semaphore* create_semaphore() override final;
        std::vector<Semaphore*> create_semaphores(uint32_t num_semaphores) override final;
        Fence* create_fence(bool signaled = false) override final;
        std::vector<Fence*> create_fences(uint32_t num_fences, bool signaled = false) override final;
        void destroy_renderpass(Renderpass* pass) override final;
        void destroy_pipeline(Pipeline* pipeline) override final;
        void destroy_texture(Image* resource) override final;
        void destroy_semaphores(const std::vector<Semaphore*>& semaphores) override final;
        void destroy_fences(const std::vector<Fence*>& fences) override final;
        CommandList* allocate_command_list(uint32_t thread_idx,
                                           QueueType needed_queue_type,
                                           CommandList::Level command_list_type) override final;
        void submit_command_list(CommandList* cmds,
                                 QueueType queue,
                                 Fence* fence_to_signal = nullptr,
                                 const std::vector<Semaphore*>& wait_semaphores = {},
                                 const std::vector<Semaphore*>& signal_semaphores = {}) override final;

    protected:
        static void set_initial_state();

        std::unique_ptr<Window> window;

        void open_window_and_create_surface(const NovaSettings::WindowOptions& options) override final;

        std::unordered_map<std::string, shaderpack::SamplerCreateInfo> samplers;
    };
} // namespace nova::renderer::rhi
