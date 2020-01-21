#include "nova_renderer/rhi/render_device.hpp"

#include <utility>

namespace nova::renderer::rhi {
    Swapchain* RenderDevice::get_swapchain() const { return swapchain; }

    mem::AllocatorHandle<>* RenderDevice::get_allocator() const {
        return &internal_allocator;
    }

    RenderDevice::RenderDevice(mem::AllocatorHandle<>& allocator,
                               NovaSettingsAccessManager& settings,
                               NovaWindow& window)
        : settings(settings),
          internal_allocator(allocator),
          window(window),
          swapchain_size(settings.settings.window.width, settings.settings.window.height) {}
} // namespace nova::renderer::rhi
