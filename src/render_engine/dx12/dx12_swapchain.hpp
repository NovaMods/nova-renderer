#pragma once

#include <wrl/client.h>

#include <dxgi1_4.h>

#include "nova_renderer/swapchain.hpp"

namespace nova::renderer::rhi {
    class DX12Swapchain final : public Swapchain {
    public:
        ~DX12Swapchain() override = default;

        void acquire_next_swapchain_image(const Semaphore* signal_semaphore) override;

        void present_current_image(const Semaphore* wait_semaphore) override;

        Image* get_image(uint32_t index) override;

    private:
        Microsoft::WRL::ComPtr<IDXGISwapChain3> swapchain;
    };
} // namespace nova::renderer::rhi
