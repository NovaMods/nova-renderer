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
        std::shared_ptr<window_t> get_window() const override final;
        result<renderpass_t*> create_renderpass(const shaderpack::render_pass_create_info_t& data) override final;
        framebuffer_t* create_framebuffer(const std::vector<resource_t*>& attachments) override final;
        pipeline_t* create_pipeline(const shaderpack::pipeline_create_info_t& data) override final;
        buffer_t* create_buffer(const buffer_create_info_t& info) override final;
        image_t* create_texture(const shaderpack::texture_create_info_t& info) override final;
        semaphore_t* create_semaphore() override final;
        std::vector<semaphore_t*> create_semaphores(uint32_t num_semaphores) override final;
        fence_t* create_fence(bool signaled = false) override final;
        std::vector<fence_t*> create_fences(uint32_t num_fences, bool signaled = false) override final;
        void destroy_renderpass(renderpass_t* pass) override final;
        void destroy_pipeline(pipeline_t* pipeline) override final;
        void destroy_texture(image_t* resource) override final;
        void destroy_semaphores(const std::vector<semaphore_t*>& semaphores) override final;
        void destroy_fences(const std::vector<fence_t*>& fences) override final;
        command_list_t* allocate_command_list(uint32_t thread_idx, queue_type needed_queue_type, command_list_t::level level) override final;
        void submit_command_list(command_list_t* cmds,
                                 queue_type queue,
                                 fence_t* fence_to_signal = nullptr,
                                 const std::vector<semaphore_t*>& wait_semaphores = {},
                                 const std::vector<semaphore_t*>& signal_semaphores = {}) override final;
        void open_window_and_create_surface(const nova_settings::window_options& options) override final;

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
