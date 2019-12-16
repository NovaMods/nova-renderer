#include "nova_renderer/frontend/ui_renderer.hpp"

namespace nova::renderer {
    void UiRenderpass::record(rhi::CommandList* cmds, FrameContext& ctx) const {
        record_pre_renderpass_barriers()
    }
} // namespace nova::renderer
