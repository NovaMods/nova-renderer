#include "nova_renderer/frontend/ui_renderer.hpp"

#include "nova_renderer/rhi/render_engine.hpp"
#include "nova_renderer/util/logger.hpp"

namespace nova::renderer {
    UiRenderpass::UiRenderpass(rhi::RenderEngine* device, const glm::vec2& framebuffer_size) {
        shaderpack::RenderPassCreateInfo rp_info = {};
        rp_info.name = UI_RENDER_PASS_NAME;
        rp_info.texture_inputs = {SCENE_OUTPUT_RT_NAME};
        rp_info.texture_outputs = {{BACKBUFFER_NAME, shaderpack::PixelFormatEnum::RGBA8, false}};

        device->create_renderpass(rp_info, framebuffer_size, *device->get_allocator())
            .map([&](rhi::Renderpass* rp) {
                renderpass = rp;
                return true;
            })
            .on_error([](const ntl::NovaError& err) { NOVA_LOG(ERROR) << "Could not create UI renderpass: " << err.to_string(); });

        writes_to_backbuffer = true;
    }

    void UiRenderpass::render_renderpass_contents(rhi::CommandList* cmds, FrameContext& ctx) { render_ui(cmds, ctx); }

    NullUiRenderpass::NullUiRenderpass(rhi::RenderEngine* device, const glm::vec2& framebuffer_size)
        : UiRenderpass(device, framebuffer_size) {}

    void NullUiRenderpass::render_ui(rhi::CommandList* /* cmds */, FrameContext& /* ctx */) {
        // Intentionally empty
    }
} // namespace nova::renderer
