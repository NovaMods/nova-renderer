#include "backbuffer_output_pass.hpp"

#include "nova_renderer/loading/shaderpack_loading.hpp"
#include "nova_renderer/nova_renderer.hpp"

namespace nova::renderer {
    RX_LOG("BackbufferOut", logger);

    struct RX_HINT_EMPTY_BASES BackbufferOutputRenderpassCreateInfo : shaderpack::RenderPassCreateInfo {
        BackbufferOutputRenderpassCreateInfo();
    };

    BackbufferOutputRenderpassCreateInfo::BackbufferOutputRenderpassCreateInfo() {
        name = BACKBUFFER_OUTPUT_RENDER_PASS_NAME;
        texture_inputs.emplace_back(UI_OUTPUT_RT_NAME);
        texture_inputs.emplace_back(SCENE_OUTPUT_RT_NAME);

        texture_outputs.emplace_back(BACKBUFFER_NAME, shaderpack::PixelFormatEnum::RGBA8, false);
    }

    RX_GLOBAL<BackbufferOutputRenderpassCreateInfo> backbuffer_output_create_info{"Nova", "BackbufferOutputCreateInfo"};

    BackbufferOutputRenderpass::BackbufferOutputRenderpass(const NovaRenderer* nova)
        : Renderpass(BACKBUFFER_OUTPUT_RENDER_PASS_NAME, true) {
    }

    const shaderpack::RenderPassCreateInfo& BackbufferOutputRenderpass::get_create_info() { return *backbuffer_output_create_info; }
} // namespace nova::renderer
