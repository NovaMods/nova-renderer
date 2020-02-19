#include "backbuffer_output_pass.hpp"

namespace nova::renderer {
    struct BackbufferOutputRenderpassCreateInfo : shaderpack::RenderPassCreateInfo {
        BackbufferOutputRenderpassCreateInfo();
    };

    BackbufferOutputRenderpassCreateInfo::BackbufferOutputRenderpassCreateInfo() {
        name = BACKBUFFER_OUTPUT_RENDER_PASS_NAME;
        texture_inputs.emplace_back(UI_OUTPUT_RT_NAME);
        texture_inputs.emplace_back(SCENE_OUTPUT_RT_NAME);

        texture_outputs.emplace_back(BACKBUFFER_NAME, shaderpack::PixelFormatEnum::RGBA8, false);
    }

    RX_GLOBAL<BackbufferOutputRenderpassCreateInfo> backbuffer_output_create_info{"Nova", "BackbufferOutputCreateInfo"};

    BackbufferOutputRenderpass::BackbufferOutputRenderpass() : Renderpass(BACKBUFFER_OUTPUT_RENDER_PASS_NAME, true) {}

    shaderpack::RenderPassCreateInfo BackbufferOutputRenderpass::get_create_info() {
        return *backbuffer_output_create_info;
    }
}
