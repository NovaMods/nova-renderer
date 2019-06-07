#include "dx12_swapchain.hpp"
#include "../../util/logger.hpp"
#include "d3dx12.h"
#include "dx12_structs.hpp"
#include "nova_renderer/render_engine.hpp"

namespace nova::renderer::rhi {
    DX12Swapchain::DX12Swapchain(RenderEngine* rhi,
                                 const IDXGIFactory4* dxgi,
                                 ID3D12Device* device,
                                 HWND window,
                                 const glm::uvec2& window_size,
                                 const uint32_t num_images,
                                 ID3D12CommandQueue* direct_command_queue)
        : num_images(num_images), window_size(window_size), rhi(rhi) {
        create_swapchain(dxgi, window, direct_command_queue);

        create_per_frame_resources(device);
    }

    uint32_t DX12Swapchain::acquire_next_swapchain_image(const Semaphore* /* signal_semaphore */) {
        const uint32_t ret_val = cur_frame_index;
        cur_frame_index++;
        return ret_val;
    }

    void DX12Swapchain::present(uint32_t /* image_idx */, const std::vector<Semaphore*> wait_semaphores) {
        swapchain->Present(0, DXGI_PRESENT_RESTRICT_TO_OUTPUT);
    }

    void DX12Swapchain::create_swapchain(const IDXGIFactory4* dxgi, HWND window, ID3D12CommandQueue* direct_command_queue) {
        DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
        swapchain_desc.Width = window_size.x;
        swapchain_desc.Height = window_size.y;
        swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        swapchain_desc.SampleDesc = {1};
        swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchain_desc.BufferCount = num_images;

        swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchain_uncast;

        HRESULT hr = dxgi->CreateSwapChainForHwnd(direct_command_queue,
                                                  window,
                                                  &swapchain_desc,
                                                  nullptr,
                                                  nullptr,
                                                  swapchain_uncast.GetAddressOf());

        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create swapchain";
            if(hr == DXGI_ERROR_INVALID_CALL) {
                NOVA_LOG(INFO) << "Invalid call - one or more of the parameters was wrong";
            } else if(hr == DXGI_STATUS_OCCLUDED) {
                NOVA_LOG(INFO) << "Fullscreen is unavailable";
            } else if(hr == E_OUTOFMEMORY) {
                NOVA_LOG(INFO) << "Out of memory. Soz bro :/";
            }
        }

        swapchain_uncast->QueryInterface(IID_PPV_ARGS(swapchain.GetAddressOf()));
    }

    void DX12Swapchain::create_per_frame_resources(ID3D12Device* device) {
        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
        rtv_heap_desc.NumDescriptors = num_images;
        rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(rtv_descriptor_heap.GetAddressOf()));

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

        for(uint32_t i = 0; i < num_images; i++) {
            Microsoft::WRL::ComPtr<ID3D12Resource> rendertarget;
            swapchain->GetBuffer(i, IID_PPV_ARGS(&rendertarget));

            swapchain_images.push_back(new DX12Image{{}, rendertarget});

            // Create the Render Target View, which binds the swapchain buffer to the RTV handle
            device->CreateRenderTargetView(rendertarget.Get(), nullptr, rtv_handle);

            framebuffers.push_back(new DX12Framebuffer{{window_size}, {rtv_handle}, false, {}, nullptr});

            fences.push_back(rhi->create_fence(true));

            // Increment the RTV handle
            rtv_handle.Offset(1, rtv_descriptor_size);
        }
    }
} // namespace nova::renderer::rhi
