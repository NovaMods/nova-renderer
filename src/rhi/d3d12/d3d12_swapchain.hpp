#pragma once

#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include "nova_renderer/rhi/swapchain.hpp"

#include "d3d12_render_device.hpp"

// Fix WinAPI cause Rex broke it
#define interface struct

namespace nova::renderer::rhi {
    class D3D12Swapchain : public Swapchain {
    public:
        explicit D3D12Swapchain(D3D12RenderDevice& rhi,
                                IDXGIFactory4* dxgi,
                                ID3D12Device* device,
                                HWND window,
                                const glm::uvec2& window_size,
                                uint32_t num_images,
                                ID3D12CommandQueue* direct_command_queue);

        ~D3D12Swapchain() override = default;

        uint8_t acquire_next_swapchain_image(rx::memory::allocator& allocator) override;

        void present(uint32_t image_idx) override;

    private:
        const uint32_t num_swapchain_images;

        uint32_t rtv_descriptor_size = 0;

        uint32_t cur_frame_index = 0;

        Microsoft::WRL::ComPtr<IDXGISwapChain3> swapchain;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap;

        D3D12RenderDevice* rhi;

        DXGI_FORMAT swapchain_format;

        void create_swapchain(IDXGIFactory4* dxgi, HWND window, ID3D12CommandQueue* direct_command_queue);

        void create_per_frame_resources(ID3D12Device* device);
    };
} // namespace nova::renderer::rhi
