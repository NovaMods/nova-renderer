#include "nova_renderer/rhi/render_device.hpp"

namespace nova::renderer::rhi {
    Swapchain* RenderDevice::get_swapchain() const { return swapchain; }

    rx::memory::allocator* RenderDevice::get_allocator() const { return internal_allocator; }

    RenderDevice::RenderDevice(NovaSettingsAccessManager& settings, NovaWindow& window, rx::memory::allocator* allocator)
        : settings(settings),
          internal_allocator(allocator),
          window(window),
          swapchain_size(settings.settings.window.width, settings.settings.window.height) {}
} // namespace nova::renderer::rhi
