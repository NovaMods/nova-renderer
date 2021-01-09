#include "nova_renderer/rhi/render_device.hpp"

#include "vulkan/vulkan_render_device.hpp"

namespace nova::renderer::rhi {
    Swapchain* RenderDevice::get_swapchain() const { return swapchain; }

    RenderDevice::RenderDevice(NovaSettingsAccessManager& settings, NovaWindow& window)
        : settings(settings),
          window(window),
          swapchain_size(settings.settings.window.width, settings.settings.window.height) {}

    std::unique_ptr<RenderDevice> create_render_device(NovaSettingsAccessManager& settings, NovaWindow& window) {
        return std::make_unique<VulkanRenderDevice>(settings, window);
    }
} // namespace nova::renderer::rhi
