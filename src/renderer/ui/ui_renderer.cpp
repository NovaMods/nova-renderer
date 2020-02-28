#include "nova_renderer/ui_renderer.hpp"

#include "nova_renderer/rhi/render_device.hpp"

namespace nova::renderer {
    struct RX_HINT_EMPTY_BASES UiRenderpassCreateInfo : shaderpack::RenderPassCreateInfo {
        UiRenderpassCreateInfo();
    };

    UiRenderpassCreateInfo::UiRenderpassCreateInfo() {
        name = UI_RENDER_PASS_NAME;
        texture_outputs.emplace_back(UI_OUTPUT_RT_NAME, shaderpack::PixelFormatEnum::RGBA8, true);
    }

    RX_GLOBAL<UiRenderpassCreateInfo> ui_create_info{"Nova", "UiRenderpassCreateInfo"};

    UiRenderpass::UiRenderpass() : Renderpass(UI_RENDER_PASS_NAME, true) {}

    void UiRenderpass::record_renderpass_contents(rhi::CommandList& cmds, FrameContext& ctx) { render_ui(cmds, ctx); }

    const shaderpack::RenderPassCreateInfo& UiRenderpass::get_create_info() {
        return *ui_create_info;
    }

    void NullUiRenderpass::render_ui(rhi::CommandList& /* cmds */, FrameContext& /* ctx */) {
        // Intentionally empty
    }
} // namespace nova::renderer
