#include "nova_renderer/render_engine.hpp"

namespace nova::renderer::rhi {
    Window& RenderDevice::get_window() const { return *window; }

    void RenderDevice::set_shaderpack_data_allocator(const bvestl::polyalloc::allocator_handle& allocator_handle) {
        shaderpack_allocator = allocator_handle;
    }

    Swapchain* RenderDevice::get_swapchain() const { return swapchain; }
} // namespace nova::renderer::rhi
