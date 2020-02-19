#include "backbuffer_output_pass.hpp"

#include "nova_renderer/nova_renderer.hpp"

namespace nova::renderer {
    struct RX_HINT_EMPTY_BASES BackbufferOutputRenderpassCreateInfo : shaderpack::RenderPassCreateInfo {
        BackbufferOutputRenderpassCreateInfo();
    };

    struct RX_HINT_EMPTY_BASES BackbufferOutputPipelineCreateInfo : shaderpack::PipelineCreateInfo {
        BackbufferOutputPipelineCreateInfo();
    };

    BackbufferOutputRenderpassCreateInfo::BackbufferOutputRenderpassCreateInfo() {
        name = BACKBUFFER_OUTPUT_RENDER_PASS_NAME;
        texture_inputs.emplace_back(UI_OUTPUT_RT_NAME);
        texture_inputs.emplace_back(SCENE_OUTPUT_RT_NAME);

        texture_outputs.emplace_back(BACKBUFFER_NAME, shaderpack::PixelFormatEnum::RGBA8, false);
    }

    BackbufferOutputPipelineCreateInfo::BackbufferOutputPipelineCreateInfo() {
        name = BACKBUFFER_OUTPUT_PIPELINE_NAME;
        pass = BACKBUFFER_OUTPUT_RENDER_PASS_NAME;

        

        const rx::string vertex_source {};

    }

    RX_GLOBAL<BackbufferOutputRenderpassCreateInfo> backbuffer_output_create_info{"Nova", "BackbufferOutputCreateInfo"};

    BackbufferOutputRenderpass::BackbufferOutputRenderpass(NovaRenderer* nova) : Renderpass(BACKBUFFER_OUTPUT_RENDER_PASS_NAME, true) {
        auto& pipeline_storage = nova->get_pipeline_storage();
    }

    const shaderpack::RenderPassCreateInfo& BackbufferOutputRenderpass::get_create_info() { return *backbuffer_output_create_info; }
} // namespace nova::renderer
