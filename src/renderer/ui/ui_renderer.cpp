#include "nova_renderer/frontend/ui_renderer.hpp"

namespace nova::renderer {
    UiRenderpass::UiRenderpass() {
        rhi::RenderPassCreateInfo rp_info = {};
    }

    void UiRenderpass::render_renderpass_contents(rhi::CommandList* cmds, FrameContext& ctx) {}

    void NullUiRenderpass::render(rhi::CommandList* cmds, FrameContext& ctx) {
        // Intentionally empty
    }

    void NullUiRenderpass::render_ui(rhi::CommandList* cmds, FrameContext& ctx) {
        // Intentionally empty
    }
} // namespace nova::renderer
