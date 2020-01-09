#include "nova_renderer/rhi/render_engine.hpp"

#include <utility>

namespace nova::renderer::rhi {
    Swapchain* RenderEngine::get_swapchain() const { return swapchain; }

    mem::AllocatorHandle<>* RenderEngine::get_allocator() {
        return &internal_allocator;
    }

    RenderEngine::RenderEngine(mem::AllocatorHandle<>& allocator,
                               NovaSettingsAccessManager& settings,
                               NovaWindow& window)
        : internal_allocator(allocator),
          settings(settings),
          window(window),
          swapchain_size(settings.settings.window.width, settings.settings.window.height) {}
} // namespace nova::renderer::rhi
