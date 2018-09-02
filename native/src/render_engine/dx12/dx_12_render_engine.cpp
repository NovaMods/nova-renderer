/*!
 * \author ddubois 
 * \date 30-Aug-18.
 */

#include "dx_12_render_engine.hpp"
#include "win32_window.hpp"
#include "dx12_command_buffer.hpp"
#include <d3d12sdklayers.h>

namespace nova {
    dx12_render_engine::dx12_render_engine(const settings &settings) : render_engine(settings), LOG(logger::instance) {
        create_device();
        create_rtv_command_queue();

        std::vector<command_buffer*> direct_buffers;
        direct_buffers.reserve(32);    // Not sure how many we need, this should be enough
        buffer_pool.emplace(static_cast<int>(command_buffer_type::GENERIC), direct_buffers);

        std::vector<command_buffer*> copy_buffers;
        copy_buffers.reserve(32);
        buffer_pool.emplace(static_cast<int>(command_buffer_type::COPY), copy_buffers);

        std::vector<command_buffer*> compute_buffers;
        compute_buffers.reserve(32);
        buffer_pool.emplace(static_cast<int>(command_buffer_type::COMPUTE), compute_buffers);
    }

    void dx12_render_engine::open_window(uint32_t width, uint32_t height) {
        window = std::make_unique<win32_window>(width, height);
        create_swapchain();
        create_render_target_descriptor_heap();
    }

    const std::string dx12_render_engine::get_engine_name() {
        return "DirectX 12";
    }

    void dx12_render_engine::create_device() {
//#ifndef NDEBUG
        ID3D12Debug* debug_controller;
        D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
        debug_controller->EnableDebugLayer();
//#endif

        NOVA_LOG(TRACE) << "Creating DX12 device";

        HRESULT hr;

        hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgi_factory));
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

        DXGI_SAMPLE_DESC sample_desc = {};
        sample_desc.Count = 1;

        DXGI_SWAP_CHAIN_DESC1 swapchain_description {};
        swapchain_description.Width = window_size.x;
        swapchain_description.Height = window_size.y;
        swapchain_description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        swapchain_description.SampleDesc = sample_desc;
        swapchain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchain_description.BufferCount = FRAME_BUFFER_COUNT;

        swapchain_description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        IDXGISwapChain1* swapchain_uncast;
        IDXGISwapChain1** pswapchain_uncast = &swapchain_uncast;
        // No target window specified in DXGI_SWAP_CHAIN_DESC, and no window associated with owning factory. [ MISCELLANEOUS ERROR #6: ]
        HRESULT hr = dxgi_factory->CreateSwapChainForHwnd(direct_command_queue, window->get_window_handle(), &swapchain_description, nullptr, nullptr, pswapchain_uncast);
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create swapchain";
            if(hr == DXGI_ERROR_INVALID_CALL) {
                NOVA_LOG(INFO) << "Invalid call - one or more of the parameters was wrong";
            } else if(hr == DXGI_STATUS_OCCLUDED) {
                NOVA_LOG(INFO) << "Fullscreen is unavaible";
            } else if(hr == E_OUTOFMEMORY) {
                NOVA_LOG(INFO) << "Out of memory. Soz bro :/";
            }

            throw std::runtime_error("Could not create swapchain");
        }

        swapchain = (IDXGISwapChain3*)swapchain_uncast;

        frame_index = swapchain->GetCurrentBackBufferIndex();
    }

    void dx12_render_engine::create_render_target_descriptor_heap() {
        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_descriptor = {};
        rtv_heap_descriptor.NumDescriptors = FRAME_BUFFER_COUNT;
        rtv_heap_descriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        // The heap isn't visible to shaders because shaders don't need to read the swapchain... but we still need a
        // descriptor heap
        rtv_heap_descriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        HRESULT hr = device->CreateDescriptorHeap(&rtv_heap_descriptor, IID_PPV_ARGS(&rtv_descriptor_heap));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create descriptor heap for the RTV";
            throw std::runtime_error("Could not create descriptor head for the RTV");
        }

        rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

        for(uint32_t i = 0; i < FRAME_BUFFER_COUNT; i++) {
            hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&rendertargets[i]));
            if(FAILED(hr)) {
                NOVA_LOG(FATAL) << "Could not create RTV for swapchain image " << i;
                throw std::runtime_error("Could not create RTV for swapchain");
            }

            // Create the Render Target View, which binds the swapchain buffer to the RTV handle
            device->CreateRenderTargetView(rendertargets[i], nullptr, rtv_handle);

            // Increment the RTV handle
            rtv_handle.Offset(1, rtv_descriptor_size);
        }
    }

    command_buffer *dx12_render_engine::allocate_command_buffer(const command_buffer_type type) {
        // TODO: The command lists and their allocators should be pooled, so we can avoid a ton of reallocation
        // Not doing that right now, but will make that change once this code is more complete
        // The Vulkan render engine should function the same way

        command_buffer* buffer = nullptr;

        auto& buffers = buffer_pool.at(static_cast<int>(type));
        if(buffers.empty()) {
            if(type == command_buffer_type::GENERIC) {
                buffer = new dx12_command_buffer<ID3D12GraphicsCommandList>(device, type);

            } else {
                buffer = new dx12_command_buffer<ID3D12CommandList>(device, type);
            }

        } else {
            buffer = buffers.back();
            buffers.pop_back();
        }

        return buffer;
    }

    void dx12_render_engine::free_command_buffer(command_buffer *buf) {
        buf->reset();

        auto type = buf->get_type();

        buffer_pool.at(static_cast<int>(type)).push_back(buf);
    }
}
