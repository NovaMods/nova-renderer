#include "nova_renderer/ui_renderer.hpp"

#include "nova_renderer/rhi/render_device.hpp"

namespace nova::renderer {
    UiRenderpass::UiRenderpass() : Renderpass(UI_RENDER_PASS_NAME, true) {}

    void UiRenderpass::execute(rhi::CommandList& cmds, FrameContext& ctx) { render_ui(cmds, ctx); }

    shaderpack::RenderPassCreateInfo UiRenderpass::get_create_info() {
        // TODO: This once was static, but caused a SIGSEGV because of being destructed too late
        //       It probably doesn't harm that this is not static anymore, but maybe it can be moved
        //       somewhere where it is just created once per program run and destructed before the
        //       rex allocator
        auto create_info = [&] {
            shaderpack::RenderPassCreateInfo new_create_info = {};
            new_create_info.name = UI_RENDER_PASS_NAME;
            new_create_info.texture_inputs.emplace_back(SCENE_OUTPUT_RT_NAME);
            new_create_info.texture_outputs.emplace_back(BACKBUFFER_NAME, shaderpack::PixelFormatEnum::RGBA8, false);

            return new_create_info;
        }();

        return create_info;
    }

    void NullUiRenderpass::render_ui(rhi::CommandList& /* cmds */, FrameContext& /* ctx */) {
        // Intentionally empty
    }
} // namespace nova::renderer
