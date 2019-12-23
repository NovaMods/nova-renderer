#include "nova_renderer/rhi/render_engine.hpp"

#include <utility>

namespace nova::renderer::rhi {
    Swapchain* RenderEngine::get_swapchain() const { return swapchain; }

    RenderEngine::RenderEngine(mem::AllocatorHandle<>& allocator,
                               NovaSettingsAccessManager& settings,
                               std::shared_ptr<NovaWindow> window)
        : internal_allocator(allocator),
          settings(settings),
          window(std::move(window)),
          swapchain_size(settings.settings.window.width, settings.settings.window.height) {}
} // namespace nova::renderer::rhi
