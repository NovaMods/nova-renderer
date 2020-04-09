#include "d3d12_swapchain.hpp"

#include <rx/core/log.h>

#include "nova_renderer/rhi/render_device.hpp"

#include "../../util/windows_utils.hpp"
#include "d3d12_structs.hpp"
#include "d3d12_utils.hpp"
#include "d3dx12.h"

namespace nova::renderer::rhi {
    RX_LOG("3D12Swapchain", logger);

    D3D12Swapchain::D3D12Swapchain(D3D12RenderDevice& rhi,
                                   IDXGIFactory4* dxgi,
                                   ID3D12Device* device,
                                   const HWND window,
                                   const glm::uvec2& window_size,
                                   const uint32_t num_images,
                                   ID3D12CommandQueue* direct_command_queue)
        : Swapchain{num_images, window_size}, num_swapchain_images{num_images}, rhi{&rhi}, swapchain_format{DXGI_FORMAT_R8G8B8A8_UNORM} {
        rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        create_swapchain(dxgi, window, direct_command_queue);

        create_per_frame_resources(device);
    }

    uint8_t D3D12Swapchain::acquire_next_swapchain_image(rx::memory::allocator& /* allocator */) {
        return static_cast<uint8_t>(swapchain->GetCurrentBackBufferIndex());
    }

    void D3D12Swapchain::present(uint32_t /* image_idx */) { swapchain->Present(0, 0); }

    void D3D12Swapchain::create_swapchain(IDXGIFactory4* dxgi, const HWND window, ID3D12CommandQueue* direct_command_queue) {
        DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
        swapchain_desc.Width = size.x;
        swapchain_desc.Height = size.y;
        swapchain_desc.Format = swapchain_format;

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
            logger->error("Could not create swapchain: %u", hr);
        }

        swapchain_uncast->QueryInterface(swapchain.GetAddressOf());
    }

    void D3D12Swapchain::create_per_frame_resources(ID3D12Device* device) {
        swapchain_images.clear();
        framebuffers.clear();
        fences.clear();

        auto& allocator = rhi->get_allocator();

        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
        rtv_heap_desc.NumDescriptors = num_images;
        rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(rtv_descriptor_heap.GetAddressOf()));

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

        for(uint32_t i = 0; i < num_images; i++) {
            Microsoft::WRL::ComPtr<ID3D12Resource> rendertarget;
            swapchain->GetBuffer(i, IID_PPV_ARGS(&rendertarget));

            const auto image_name = rx::string::format("Swapchain image %d", i);
            rendertarget->SetName(reinterpret_cast<LPCWSTR>(image_name.to_utf16().data()));

            auto image = rx::make_ptr<D3D12Image>(allocator, RhiImage{RhiResource{ResourceType::Image, true}, false}, swapchain_format, nullptr, rendertarget);
            swapchain_images.push_back(rx::utility::move(image));

            // Create the Render Target View, which binds the swapchain buffer to the RTV handle
            device->CreateRenderTargetView(rendertarget.Get(), nullptr, rtv_handle);
            logger->info("Created RTV descriptor %x for swapchain image %u",
                         rtv_handle.ptr,
                         reinterpret_cast<uint64_t>(rendertarget.Get()));

            auto framebuffer = rx::make_ptr<D3D12Framebuffer>(allocator);
            framebuffer->size = size;
            framebuffer->num_attachments = 1;
            framebuffer->render_target_descriptors = {allocator, rx::array{rtv_handle}};

            framebuffers.push_back(rx::utility::move(framebuffer));

            fences.push_back(rhi->create_fence(true, rhi->get_allocator()));

            // Increment the RTV handle
            rtv_handle = rtv_handle.Offset(1, rtv_descriptor_size);
        }
    }
} // namespace nova::renderer::rhi
