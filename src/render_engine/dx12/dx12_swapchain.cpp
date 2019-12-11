#include "dx12_swapchain.hpp"

#include "nova_renderer/rhi/render_engine.hpp"

#include "../../util/logger.hpp"
#include "../../util/windows_utils.hpp"
#include "d3dx12.h"
#include "dx12_structs.hpp"
#include "dx12_utils.hpp"

namespace nova::renderer::rhi {
    DX12Swapchain::DX12Swapchain(RenderEngine* rhi,
                                 IDXGIFactory4* dxgi,
                                 ID3D12Device* device,
                                 HWND window,
                                 const glm::uvec2& window_size,
                                 const uint32_t num_images,
                                 ID3D12CommandQueue* direct_command_queue)
        : Swapchain(num_images, window_size), rhi(rhi) {
        rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        create_swapchain(dxgi, window, direct_command_queue);

        create_per_frame_resources(device);
    }

    uint8_t DX12Swapchain::acquire_next_swapchain_image() { return static_cast<uint8_t>(swapchain->GetCurrentBackBufferIndex()); }

    void DX12Swapchain::present(uint32_t /* image_idx */) { swapchain->Present(0, DXGI_PRESENT_RESTRICT_TO_OUTPUT); }

    void DX12Swapchain::create_swapchain(IDXGIFactory4* dxgi, const HWND window, ID3D12CommandQueue* direct_command_queue) {
        DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
        swapchain_desc.Width = size.x;
        swapchain_desc.Height = size.y;
        swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        swapchain_desc.SampleDesc = {1};
        swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchain_desc.BufferCount = num_images;

        swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchain_uncast;

        const HRESULT hr = dxgi->CreateSwapChainForHwnd(direct_command_queue,
                                                        window,
                                                        &swapchain_desc,
                                                        nullptr,
                                                        nullptr,
                                                        swapchain_uncast.GetAddressOf());

        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create swapchain: " << to_string(hr);
        }

        swapchain_uncast->QueryInterface(IID_PPV_ARGS(swapchain.GetAddressOf()));
    }

    void DX12Swapchain::create_per_frame_resources(ID3D12Device* device) {
        swapchain_images.clear();
        framebuffers.clear();
        fences.clear();

        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
        rtv_heap_desc.NumDescriptors = num_images;
        rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(rtv_descriptor_heap.GetAddressOf()));

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

        for(uint32_t i = 0; i < num_images; i++) {
            Microsoft::WRL::ComPtr<ID3D12Resource> rendertarget;
            swapchain->GetBuffer(i, IID_PPV_ARGS(&rendertarget));

            const auto image_name = fmt::format(fmt("Swapchain image {:d}"), i);
            rendertarget->SetName(s2ws(image_name).c_str());

            swapchain_images.push_back(new DX12Image{{{ResourceType::Image, true}, false}, rendertarget});

            // Create the Render Target View, which binds the swapchain buffer to the RTV handle
            device->CreateRenderTargetView(rendertarget.Get(), nullptr, rtv_handle);
            NOVA_LOG(INFO) << "Created RTV descriptor " << rtv_handle.ptr << " for swapchain image "
                           << reinterpret_cast<uint64_t>(rendertarget.Get());

            auto* framebuffer = new DX12Framebuffer;
            framebuffer->size = size;
            framebuffer->num_attachments = 1;
            framebuffer->rtv_descriptors = {rtv_handle};

            framebuffers.push_back(framebuffer);

            fences.push_back(rhi->create_fence(true));

            // Increment the RTV handle
            rtv_handle = rtv_handle.Offset(1, rtv_descriptor_size);
        }
    }
} // namespace nova::renderer::rhi
