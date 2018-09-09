/*!
 * \author ddubois 
 * \date 30-Aug-18.
 */

#include <windows.h>

#include "dx12_render_engine.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_opaque_types.hpp"
#include <d3d12sdklayers.h>

namespace nova {
    dx12_render_engine::dx12_render_engine(const settings &settings) : render_engine(settings) {
        create_device();
        create_rtv_command_queue();
        create_frame_end_fences();

        std::vector<command_buffer_base*> direct_buffers;
        direct_buffers.reserve(32);    // Not sure how many we need, this should be enough
        buffer_pool.emplace(static_cast<int>(command_buffer_type::GENERIC), direct_buffers);

        std::vector<command_buffer_base*> copy_buffers;
        copy_buffers.reserve(32);
        buffer_pool.emplace(static_cast<int>(command_buffer_type::COPY), copy_buffers);

        std::vector<command_buffer_base*> compute_buffers;
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
        ComPtr<ID3D12Debug> debug_controller;
        D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
        debug_controller->EnableDebugLayer();
//#endif

        NOVA_LOG(TRACE) << "Creating DX12 device";

        HRESULT hr;

        hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgi_factory));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create DXGI Factory";
            throw render_engine_initialization_exception("Could not create DXGI Factory");
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
            NOVA_LOG(FATAL) << "Could not find a GPU that supports DX12";
            throw render_engine_initialization_exception("Could not find a GPU that supports DX12");
        }

        NOVA_LOG(TRACE) << "Adapter found";

        hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create Dx12 device";
            throw render_engine_initialization_exception("Could not create Dx12 device");
        }
    }

    void dx12_render_engine::create_rtv_command_queue() {
        D3D12_COMMAND_QUEUE_DESC rtv_queue_desc = {};

        HRESULT hr = device->CreateCommandQueue(&rtv_queue_desc, IID_PPV_ARGS(&direct_command_queue));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create main command queue";
            throw render_engine_initialization_exception("Could not create main command queue");
        }
    }

    void dx12_render_engine::create_swapchain() {
        if(!window) {
            NOVA_LOG(FATAL) << "Cannot initialize the swapchain before the window!";
            throw render_engine_initialization_exception("Cannot initialize the swapchain before the window");
        }

        const auto& window_size = window->get_window_size();

        DXGI_SAMPLE_DESC sample_desc = {};
        sample_desc.Count = 1;

        DXGI_SWAP_CHAIN_DESC1 swapchain_description {};
        swapchain_description.Width = window_size.width;
        swapchain_description.Height = window_size.height;
        swapchain_description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        swapchain_description.SampleDesc = sample_desc;
        swapchain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchain_description.BufferCount = FRAME_BUFFER_COUNT;

        swapchain_description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        ComPtr<IDXGISwapChain1> swapchain_uncast;
        // No target window specified in DXGI_SWAP_CHAIN_DESC, and no window associated with owning factory. [ MISCELLANEOUS ERROR #6: ]
        HRESULT hr = dxgi_factory->CreateSwapChainForHwnd(direct_command_queue.Get(), window->get_window_handle(), &swapchain_description, nullptr, nullptr, &swapchain_uncast);
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create swapchain";
            if(hr == DXGI_ERROR_INVALID_CALL) {
                NOVA_LOG(INFO) << "Invalid call - one or more of the parameters was wrong";
            } else if(hr == DXGI_STATUS_OCCLUDED) {
                NOVA_LOG(INFO) << "Fullscreen is unavaible";
            } else if(hr == E_OUTOFMEMORY) {
                NOVA_LOG(INFO) << "Out of memory. Soz bro :/";
            }

            throw render_engine_initialization_exception("Could not create swapchain");
        }

        swapchain_uncast->QueryInterface(IID_PPV_ARGS(&swapchain));
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
            throw render_engine_initialization_exception("Could not create descriptor head for the RTV");
        }

        rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

        for(uint32_t i = 0; i < FRAME_BUFFER_COUNT; i++) {
            hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&rendertargets[i]));
            if(FAILED(hr)) {
                NOVA_LOG(FATAL) << "Could not create RTV for swapchain image " << i;
                throw render_engine_initialization_exception("Could not create RTV for swapchain");
            }

            // Create the Render Target View, which binds the swapchain buffer to the RTV handle
            device->CreateRenderTargetView(rendertargets[i].Get(), nullptr, rtv_handle);

            // Increment the RTV handle
            rtv_handle.Offset(1, rtv_descriptor_size);
        }
    }

    std::shared_ptr<iwindow> dx12_render_engine::get_window() const {
        return window;
    }

    void dx12_render_engine::render_frame() {
        gfx_command_list present_commands = get_graphics_command_list();

        HRESULT hr = present_commands.allocator->Reset();
        if(FAILED(hr)) {
            NOVA_LOG(WARN) << "Could not reset command list allocator, memory usage will likely increase dramatically";
        }

        present_commands.list->Reset(present_commands.allocator.Get(), nullptr);

        CD3DX12_RESOURCE_BARRIER to_render_target = CD3DX12_RESOURCE_BARRIER::Transition(rendertargets[frame_index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        present_commands.list->ResourceBarrier(1, &to_render_target);

        CD3DX12_CPU_DESCRIPTOR_HANDLE current_framebuffer_rtv(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), frame_index, rtv_descriptor_size);
        present_commands.list->OMSetRenderTargets(1, &current_framebuffer_rtv, false, nullptr);

        glm::vec4 clear_color(0, 0.2f, 0.4f, 1.0f);
        present_commands.list->ClearRenderTargetView(current_framebuffer_rtv, &clear_color.x, 0, nullptr);

        CD3DX12_RESOURCE_BARRIER to_presentable = CD3DX12_RESOURCE_BARRIER::Transition(rendertargets[frame_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        present_commands.list->ResourceBarrier(1, &to_presentable);

        present_commands.list->Close();

        ID3D12CommandList* commands[] = {present_commands.list.Get()};
        direct_command_queue->ExecuteCommandLists(1, commands);

        engine->present_swapchain_image();

        for(auto& typed_fence : fences) {
            engine->wait_for_fence(typed_fence.second.get(), std::numeric_limits<uint64_t>::max());
            engine->free_fence(std::move(typed_fence.second));
        }

        engine->free_command_buffer(std::move(buffer));

        frame_index = engine->get_current_swapchain_index();
    }

    command_list dx12_render_engine::allocate_command_list(D3D12_COMMAND_LIST_TYPE command_list_type) {
        ComPtr<ID3D12CommandAllocator> allocator;
        HRESULT hr = device->CreateCommandAllocator(command_list_type, IID_PPV_ARGS(&allocator));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create command buffer";
            throw std::runtime_error("Could not create command buffer");
        }

        ComPtr<ID3D12CommandList> list;
        hr = device->CreateCommandList(0, command_list_type, allocator.Get(), nullptr, IID_PPV_ARGS(&list));
        if(FAILED(hr)) {
            NOVA_LOG(ERROR) << "Could not create a command list of type " << (uint32_t)command_list_type;
            throw std::runtime_error("Could not create command list");
        }

        return {list, allocator};
    }

    gfx_command_list dx12_render_engine::get_graphics_command_list() {
        command_list new_list;
        ComPtr<ID3D12GraphicsCommandList> graphics_list;
        auto& buffers = buffer_pool.at(static_cast<int>(command_buffer_type::GENERIC));
        if(buffers.empty()) {
            new_list = allocate_command_list(D3D12_COMMAND_LIST_TYPE_DIRECT);

        } else {
            new_list = buffers.back();
            buffers.pop_back();
        }
        new_list.list->QueryInterface(IID_PPV_ARGS(&graphics_list));

        return {graphics_list, new_list.allocator};
    }

    void dx12_render_engine::present_swapchain_image() {
        swapchain->Present(0, 0);
    }

    std::shared_ptr<iframebuffer> dx12_render_engine::get_swapchain_framebuffer(uint32_t frame_index) const {
        std::shared_ptr<iframebuffer> framebuffer = std::make_shared<iframebuffer>();
        CD3DX12_CPU_DESCRIPTOR_HANDLE current_framebuffer_rtv(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), frame_index, rtv_descriptor_size);
        framebuffer->color_attachments.push_back(current_framebuffer_rtv);
        return framebuffer;
    }

    void dx12_render_engine::execute_command_buffers(const std::vector<command_buffer_base*> &buffers) {
        std::vector<ID3D12CommandList*> lists;
        lists.reserve(buffers.size());

        for(command_buffer_base* buffer : buffers) {
            auto* dx12_buffer = dynamic_cast<dx12_command_buffer*>(buffer);
            lists.push_back(dx12_buffer->command_list.Get());
        }

        direct_command_queue->ExecuteCommandLists(lists.size(), lists.data());

        // We need to tell the queue to signal our fences after we tell it to execute our command lists
        for(auto* buffer : buffers) {
            auto* dx12_buffer = dynamic_cast<dx12_command_buffer*>(buffer);
            direct_command_queue->Signal(dx12_buffer->fence.Get(), dx12_buffer->fence_value);
            dx12_buffer->fence_value++;
        }
    }

    std::shared_ptr<iresource> dx12_render_engine::get_swapchain_image(uint32_t frame_index) const {
        std::shared_ptr<iresource> resource = std::make_shared<iresource>();
        resource->descriptor = rendertargets[frame_index];
        return resource;
    }

    uint32_t dx12_render_engine::get_current_swapchain_index() const {
        return swapchain->GetCurrentBackBufferIndex();
    }
}
