#include "backbuffer_output_pass.hpp"

#include "nova_renderer/loading/shaderpack_loading.hpp"
#include "nova_renderer/nova_renderer.hpp"
#include "nova_renderer/rhi/pipeline_create_info.hpp"

namespace nova::renderer {
    RX_LOG("BackbufferOut", logger);

    struct RX_HINT_EMPTY_BASES BackbufferOutputRenderpassCreateInfo : shaderpack::RenderPassCreateInfo {
        BackbufferOutputRenderpassCreateInfo();
    };

    struct RX_HINT_EMPTY_BASES BackbufferOutputPipelineCreateInfo : PipelineStateCreateInfo {
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

        const rx::string vertex_source{R"(
            struct VsInput {
                float2 position : POSITION;
            };

            struct VsOutput {
                float4 position : SV_POSITION;
                float2 uv : TEXCOORD;

            };

            VsOutput main(VsInput input) {
                VsOutput output;
                output.position = float4(input.position, 0, 1);
                output.uv = input.position; // TODO: Figure out if I should flip the texture coordinates

                return output;
            })"};
        const auto& vertex_spirv = shaderpack::compile_shader(vertex_source, rhi::ShaderStage::Vertex, rhi::ShaderLanguage::Hlsl);
        vertex_shader = {"/nova/shaders/backbuffer_output.vertex.hlsl", vertex_spirv};

        const rx::string pixel_source{R"(
            [[vk::binding(0, 0)]]
            Texture2D ui_output : register(t0);

            [[vk::binding(1, 0)]]
            Texture2D scene_output : register(t1);

            [[vk::binding(2, 0)]]
            SamplerState sampler : register(s0);

            struct VsOutput {
                float4 position : SV_POSITION;
                float2 us : TEXCOORD;
            };

            float3 main(VsOutput input) : SV_Target {
                float4 ui_color = ui_output.Sample(sampler, input.uv);
                float4 scene_color = scene_output.Sample(sampler, input.uv);

                float3 combined_color = lerp(ui_color.rgb, scene_color.rgb, ui_color.a);

                return combined_color;
            })"};
        const auto& pixel_spirv = shaderpack::compile_shader(pixel_source, rhi::ShaderStage::Fragment, rhi::ShaderLanguage::Hlsl);
        fragment_shader = {"/nova/shaders/backbuffer_output.pixel.hlsl", pixel_spirv};
    }

    RX_GLOBAL<BackbufferOutputRenderpassCreateInfo> backbuffer_output_create_info{"Nova", "BackbufferOutputCreateInfo"};

    RX_GLOBAL<BackbufferOutputPipelineCreateInfo> backbuffer_output_pipeline_create_info{"Nova", "BackbufferOutputPipelineCreateInfo"};

    BackbufferOutputRenderpass::BackbufferOutputRenderpass(const NovaRenderer* nova)
        : Renderpass(BACKBUFFER_OUTPUT_RENDER_PASS_NAME, true) {
        auto& pipeline_storage = nova->get_pipeline_storage();
        if(!pipeline_storage.create_pipeline(*backbuffer_output_pipeline_create_info)) {
            logger(rx::log::level::k_error, "Could not create backbuffer output pipeline");
            return;
        }


    }

    const shaderpack::RenderPassCreateInfo& BackbufferOutputRenderpass::get_create_info() { return *backbuffer_output_create_info; }
} // namespace nova::renderer
