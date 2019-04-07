/*!
 * \author ddubois
 * \date 03-Apr-19.
 */

#include <d3d12sdklayers.h>

#include "../../util/logger.hpp"
#include "d3d12_render_engine.hpp"
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

namespace nova::renderer {
    d3d12_render_engine::d3d12_render_engine(nova_settings& settings) : render_engine(settings) { create_device(); }

    std::shared_ptr<window_t> d3d12_render_engine::get_window() const { return window; }

    result<renderpass_t*> d3d12_render_engine::create_renderpass(const render_pass_create_info_t& data) {
        return result<renderpass_t*>(new d3d12_renderpass_t);
    }

    framebuffer_t* d3d12_render_engine::create_framebuffer(const std::vector<resource_t*>& attachments) { return nullptr; }

    pipeline_t* d3d12_render_engine::create_pipeline(const pipeline_create_info_t& data) { return nullptr; }

    resource_t* d3d12_render_engine::create_buffer(const buffer_create_info_t& info) { return nullptr; }

    resource_t* d3d12_render_engine::create_texture(const texture2d_create_info_t& info) { return nullptr; }

    semaphore_t* d3d12_render_engine::create_semaphore() { return nullptr; }

    std::vector<semaphore_t*> d3d12_render_engine::create_semaphores(uint32_t num_semaphores) { return std::vector<semaphore_t*>(); }

    fence_t* d3d12_render_engine::create_fence(bool signaled = false) { return nullptr; }

    std::vector<fence_t*> d3d12_render_engine::create_fences(uint32_t num_fences, bool signaled = false) { return std::vector<fence_t*>(); }

    void d3d12_render_engine::destroy_renderpass(renderpass_t* pass) {}

    void d3d12_render_engine::destroy_pipeline(pipeline_t* pipeline) {}

    void d3d12_render_engine::destroy_resource(resource_t* resource) {}

    void d3d12_render_engine::destroy_semaphores(const std::vector<semaphore_t*>& semaphores) {}

    void d3d12_render_engine::destroy_fences(const std::vector<fence_t*>& fences) {}

    command_list_t* d3d12_render_engine::allocate_command_list(uint32_t thread_idx,
                                                               queue_type needed_queue_type,
                                                               command_list_t::level level) {
        return nullptr;
    }

    void d3d12_render_engine::submit_command_list(command_list_t* cmds,
                                                  queue_type queue,
                                                  fence_t* fence_to_signal,
                                                  const std::vector<semaphore_t*>& wait_semaphores,
                                                  const std::vector<semaphore_t*>& signal_semaphores) {}

    void d3d12_render_engine::open_window_and_create_surface(const nova_settings::window_options& options) {}

    std::optional<nova_error> d3d12_render_engine::create_device() {
        if(settings.debug.enabled && settings.debug.enable_validation_layers) {
            ComPtr<ID3D12Debug> debug_controller;
            D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
            debug_controller->EnableDebugLayer();
        }

        NOVA_LOG(TRACE) << "Creating DX12 device";

        HRESULT hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgi_factory));
        if(FAILED(hr)) {
            return std::make_optional(nova_error("Could not create DXGI Factory"));
        }
        NOVA_LOG(TRACE) << "Device created";

        ComPtr<IDXGIAdapter1> adapter;

        uint32_t adapter_index = 0;
        bool adapter_found = false;

        while(dxgi_factory->EnumAdapters1(adapter_index, &adapter) != DXGI_ERROR_NOT_FOUND) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                // Ignore software devices
                adapter_index++;
                continue;
            }

            // Direct3D 12 is feature level 11.
            //
            // cool
            hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
            if(SUCCEEDED(hr)) {
                adapter_found = true;
                break;
            }

            adapter_index++;
        }

        if(!adapter_found) {
            return std::make_optional(nova_error("Could not find a GPU that supports DX12"));
        }

        NOVA_LOG(TRACE) << "Adapter found";

        hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
        if(FAILED(hr)) {
            return std::make_optional(nova_error("Could not create DX12 device"));
        }
    }
} // namespace nova::renderer
