#include "nova_renderer/rhi/render_engine.hpp"

#include <utility>

namespace nova::renderer::rhi {
    Swapchain* RenderEngine::get_swapchain() const { return swapchain; }

    mem::AllocatorHandle<>* RenderEngine::get_allocator() const {
        return &internal_allocator;
    }

    RenderEngine::RenderEngine(mem::AllocatorHandle<>& allocator,
                               NovaSettingsAccessManager& settings,
                               NovaWindow& window)
        : settings(settings),
          internal_allocator(allocator),
          window(window),
          swapchain_size(settings.settings.window.width, settings.settings.window.height) {}
} // namespace nova::renderer::rhi
