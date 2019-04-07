/*!
 * \author ddubois
 * \date 03-Apr-19.
 */

#pragma once

#include <nova_renderer/render_engine_t.hpp>

#include <d3d12.h>

#include <DirectXMath.h>
#include <dxgi1_4.h>
#include <wrl.h>

namespace nova::renderer::rhi {
    struct d3d12_renderpass_t : renderpass_t {};

    /*!
     * \brief D3D12 implementation of a render engine
     */
    class d3d12_render_engine : public render_engine_t {
    public:
        d3d12_render_engine(nova_settings& settings);

        d3d12_render_engine(d3d12_render_engine&& old) noexcept = delete;
        d3d12_render_engine& operator=(d3d12_render_engine&& old) noexcept = delete;

        d3d12_render_engine(const d3d12_render_engine& other) = delete;
        d3d12_render_engine& operator=(const d3d12_render_engine& other) = delete;

        // Inherited via render_engine
        std::shared_ptr<window_t> get_window() const override;
        result<renderpass_t*> create_renderpass(const render_pass_create_info_t& data) override;
        framebuffer_t* create_framebuffer(const std::vector<resource_t*>& attachments) override;
        pipeline_t* create_pipeline(const pipeline_create_info_t& data) override;
        resource_t* create_buffer(const buffer_create_info_t& info) override;
        resource_t* create_texture(const texture2d_create_info_t& info) override;
        semaphore_t* create_semaphore() override;
        std::vector<semaphore_t*> create_semaphores(uint32_t num_semaphores) override;
        fence_t* create_fence(bool signaled = false) override;
        std::vector<fence_t*> create_fences(uint32_t num_fences, bool signaled = false) override;
        void destroy_renderpass(renderpass_t* pass) override;
        void destroy_pipeline(pipeline_t* pipeline) override;
        void destroy_resource(resource_t* resource) override;
        void destroy_semaphores(const std::vector<semaphore_t*>& semaphores) override;
        void destroy_fences(const std::vector<fence_t*>& fences) override;
        command_list_t* allocate_command_list(uint32_t thread_idx, queue_type needed_queue_type, command_list_t::level level) override;
        void submit_command_list(command_list_t* cmds,
                                 queue_type queue,
                                 fence_t* fence_to_signal = nullptr,
                                 const std::vector<semaphore_t*>& wait_semaphores = {},
                                 const std::vector<semaphore_t*>& signal_semaphores = {}) override;
        void open_window_and_create_surface(const nova_settings::window_options& options) override;

    private:
        Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory;

        Microsoft::WRL::ComPtr<ID3D12Device> device; // direct3d device

        Microsoft::WRL::ComPtr<IDXGISwapChain3> swapchain; // swapchain used to switch between render targets

        Microsoft::WRL::ComPtr<ID3D12CommandQueue> direct_command_queue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> compute_command_queue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> copy_command_queue;

#pragma region Initialization
        void create_device();

        void create_queues();
#pragma endregion
    };
} // namespace nova::renderer
