#include "nova_renderer/frontend/ui_renderer.hpp"

#include "nova_renderer/rhi/render_engine.hpp"

namespace nova::renderer {
    UiRenderpass::UiRenderpass() : Renderpass(UI_RENDER_PASS_NAME, true) {}

    void UiRenderpass::render_renderpass_contents(rhi::CommandList& cmds, FrameContext& ctx) { render_ui(cmds, ctx); }

    shaderpack::RenderPassCreateInfo UiRenderpass::get_create_info() {
        static auto create_info = [&] {
            shaderpack::RenderPassCreateInfo new_create_info = {};
            new_create_info.name = UI_RENDER_PASS_NAME;
            new_create_info.texture_inputs = {SCENE_OUTPUT_RT_NAME};
            new_create_info.texture_outputs = {{BACKBUFFER_NAME, shaderpack::PixelFormatEnum::RGBA8, false}};

            return new_create_info;
        }();

        return create_info;
    }

    void NullUiRenderpass::render_ui(rhi::CommandList& /* cmds */, FrameContext& /* ctx */) {
        // Intentionally empty
    }
} // namespace nova::renderer
