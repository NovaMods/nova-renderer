#include "nova_renderer/frontend/ui_renderer.hpp"

namespace nova::renderer {
    void UiRenderpass::render_renderpass_contents(rhi::CommandList* cmds, FrameContext& ctx) {}

    void NullUiRenderpass::render(rhi::CommandList* cmds, FrameContext& ctx) {
        // Intentionally empty
    }

    void NullUiRenderpass::render_ui(rhi::CommandList* cmds, FrameContext& ctx) const {
        // Intentionally empty
    }
} // namespace nova::renderer
