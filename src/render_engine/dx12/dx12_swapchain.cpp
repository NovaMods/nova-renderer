#include "dx12_swapchain.hpp"

namespace nova::renderer::rhi {
    void DX12Swapchain::acquire_next_swapchain_image(const Semaphore* signal_semaphore) {}

    void DX12Swapchain::present_current_image(const Semaphore* wait_semaphore) {}

    Image* DX12Swapchain::get_image(const uint32_t index) {}
} // namespace nova::renderer::rhi
