/*!
 * \author ddubois 
 * \date 30-Aug-18.
 */

#include "dx_12_render_engine.hpp"
#include "win32_window.hpp"

namespace nova {
    dx12_render_engine::dx12_render_engine(const settings &settings) : render_engine(settings), LOG(logger::instance) {
        create_device();
        create_rtv_command_queue();
    }

    void dx12_render_engine::open_window(uint32_t width, uint32_t height) {
        window = std::make_unique<win32_window>(width, height);
        create_swapchain();
    }

    const std::string dx12_render_engine::get_engine_name() {
        return "DirectX 12";
    }

    void dx12_render_engine::create_device() {
        NOVA_LOG(TRACE) << "Creating DX12 device";

        HRESULT hr;

        hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create DXGI Factory";
            throw std::runtime_error("Could not create DXGI Factory");
        }
        NOVA_LOG(TRACE) << "Device created";

        IDXGIAdapter1* adapter;

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
            hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
            if(SUCCEEDED(hr)) {
                adapter_found = true;
                break;
            }

            adapter_index++;
        }

        if(!adapter_found) {
            NOVA_LOG(FATAL) << "Could not find a GPU that supports DX12";
            throw std::runtime_error("Could not find a GPU that supports DX12");
        }

        NOVA_LOG(TRACE) << "Adapter found";

        hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create Dx12 device";
            throw std::runtime_error("Could not create Dx12 device");
        }
    }

    void dx12_render_engine::create_rtv_command_queue() {
        D3D12_COMMAND_QUEUE_DESC rtv_queue_desc = {};

        HRESULT hr = device->CreateCommandQueue(&rtv_queue_desc, IID_PPV_ARGS(&direct_command_queue));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create main command queue";
            throw std::runtime_error("Could not create main command queue");
        }
    }

    void dx12_render_engine::create_swapchain() {
        if(!window) {
            NOVA_LOG(FATAL) << "Cannot initialize the swapchain before the window!";
            throw std::runtime_error("Cannot initialize the swapchain before the window");
        }

        const auto& window_size = window->get_size();

        DXGI_MODE_DESC backbuffer_description = {};
        backbuffer_description.Width = window_size.x;
        backbuffer_description.Height = window_size.y;
        backbuffer_description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        DXGI_SAMPLE_DESC sample_desc = {};
        sample_desc.Count = 1;

        DXGI_SWAP_CHAIN_DESC swapchain_description {};
        swapchain_description.BufferCount = frameBufferCount;
        swapchain_description.BufferDesc = backbuffer_description;
        swapchain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchain_description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchain_description.OutputWindow = window->get_window_handle();
        swapchain_description.SampleDesc = sample_desc;
        swapchain_description.Windowed = true;

        IDXGISwapChain* swapchain_uncast;
        dxgi_factory->CreateSwapChain(direct_command_queue, &swapchain_description, &swapchain_uncast);

        swapchain = dynamic_cast<IDXGISwapChain3*>(swapchain_uncast);

        frame_index = swapchain->GetCurrentBackBufferIndex();
    }
}
