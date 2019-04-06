/*!
 * \author ddubois
 * \date 03-Apr-19.
 */

#pragma once

#include <nova_renderer/render_engine.hpp>

namespace nova::renderer {
    struct d3d12_renderpass_t : renderpass_t{};

    /*!
     * \brief D3D12 implmentation of a render engine
     */
    class d3d12_render_engine : public render_engine {
    public:
        d3d12_render_engine(nova_settings& settings);

        d3d12_render_engine(d3d12_render_engine&& old) noexcept = delete;
        d3d12_render_engine& operator=(d3d12_render_engine&& old) noexcept = delete;

        d3d12_render_engine(const d3d12_render_engine& other) = delete;
        d3d12_render_engine& operator=(const d3d12_render_engine& other) = delete;

        // Inherited via render_engine
        virtual std::shared_ptr<window> get_window() const override;
        virtual result<renderpass_t*> create_renderpass(const render_pass_create_info_t& data) override;
        virtual framebuffer_t* create_framebuffer(const std::vector<resource_t*>& attachments) override;
        virtual pipeline_t* create_pipeline(const pipeline_create_info_t& data) override;
        virtual resource_t* create_buffer(const buffer_create_info_t& info) override;
        virtual resource_t* create_texture(const texture2d_create_info_t& info) override;
        virtual semaphore_t* create_semaphore() override;
        virtual std::vector<semaphore_t*> create_semaphores(uint32_t num_semaphores) override;
        virtual fence_t* create_fence(bool signaled = false) override;
        virtual std::vector<fence_t*> create_fences(uint32_t num_fences, bool signaled = false) override;
        virtual void destroy_renderpass(renderpass_t* pass) override;
        virtual void destroy_pipeline(pipeline_t* pipeline) override;
        virtual void destroy_resource(resource_t* resource) override;
        virtual void destroy_semaphores(const std::vector<semaphore_t*>& semaphores) override;
        virtual void destroy_fences(const std::vector<fence_t*>& fences) override;
        virtual command_list_t* allocate_command_list(uint32_t thread_idx,
                                                      queue_type needed_queue_type,
                                                      command_list_t::level level) override;
        virtual void submit_command_list(command_list_t* cmds,
                                         queue_type queue,
                                         fence_t* fence_to_signal = nullptr,
                                         const std::vector<semaphore_t*>& wait_semaphores = {},
                                         const std::vector<semaphore_t*>& signal_semaphores = {}) override;
        virtual void open_window_and_create_surface(const nova_settings::window_options& options) override;
    };
} // namespace nova::renderer
