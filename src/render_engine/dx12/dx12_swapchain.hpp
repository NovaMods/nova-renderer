#pragma once

#pragma warning(push, 0)
#include <d3d12.h>
#include <dxgi1_4.h>
#include <glm/vec2.hpp>
#include <wrl/client.h>
#pragma warning(pop)

#include "nova_renderer/swapchain.hpp"

namespace nova::renderer::rhi {
    class RenderEngine;

    class DX12Swapchain final : public Swapchain {
    public:
        DX12Swapchain(RenderEngine* rhi,
                      IDXGIFactory4* dxgi,
                      ID3D12Device* device,
                      HWND window,
                      const glm::uvec2& window_size,
                      uint32_t num_images,
                      ID3D12CommandQueue* direct_command_queue);

        ~DX12Swapchain() override = default;

#pragma region Swapchain implementation
        uint8_t acquire_next_swapchain_image() override;

        void present(uint32_t image_idx) override;
#pragma endregion

    private:
        uint32_t rtv_descriptor_size = 0;

        uint32_t cur_frame_index = 0;

        RenderEngine* rhi;

        Microsoft::WRL::ComPtr<IDXGISwapChain3> swapchain;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap;

        void create_swapchain(IDXGIFactory4* dxgi, HWND window, ID3D12CommandQueue* direct_command_queue);

        void create_per_frame_resources(ID3D12Device* device);
    };
} // namespace nova::renderer::rhi
