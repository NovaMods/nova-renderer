#include "nova_renderer/render_engine.hpp"

namespace nova::renderer::rhi {
    void RenderEngine::set_shaderpack_data_allocator(const bvestl::polyalloc::allocator_handle& allocator_handle) {
        shaderpack_allocator = allocator_handle;
    }

    Swapchain* RenderEngine::get_swapchain() const { return swapchain; }

    RenderEngine::RenderEngine(bvestl::polyalloc::Allocator* allocator,
                               NovaSettingsAccessManager& settings,
                               const std::shared_ptr<Window>& window)
        : settings(settings),
          window(window),
          swapchain_size(settings.settings.window.width, settings.settings.window.height),
          shaderpack_allocator(allocator) {}
} // namespace nova::renderer::rhi
