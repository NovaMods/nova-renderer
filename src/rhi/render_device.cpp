#include "nova_renderer/rhi/render_device.hpp"

#include "d3d12/d3d12_render_device.hpp"
#include "vulkan/vulkan_render_device.hpp"

namespace nova::renderer::rhi {
    Swapchain* RenderDevice::get_swapchain() const { return swapchain.get(); }

    rx::memory::allocator& RenderDevice::get_allocator() const { return internal_allocator; }

    RenderDevice::RenderDevice(NovaSettingsAccessManager& settings, NovaWindow& window, rx::memory::allocator& allocator)
        : settings(settings),
          internal_allocator(allocator),
          window(window),
          swapchain_size(settings.settings.window.width, settings.settings.window.height) {}

    rx::ptr<RenderDevice> create_render_device(NovaSettingsAccessManager& settings, NovaWindow& window, rx::memory::allocator& allocator) {
        switch(settings->api) {
            case Api::D3D12:
                return rx::make_ptr<D3D12RenderDevice>(allocator, settings, window, allocator);

            case Api::Vulkan:
                return rx::make_ptr<VulkanRenderDevice>(allocator, settings, window, allocator);
        }
    }
} // namespace nova::renderer::rhi
