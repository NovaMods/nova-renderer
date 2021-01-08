#include "nova_renderer/renderpack_data_conversions.hpp"

#include "nova_renderer/rendergraph.hpp"
#include "nova_renderer/renderpack_data.hpp"
#include "nova_renderer/rhi/rhi_enums.hpp"

#include "rx/core/log.h"
#include "spirv_glsl.hpp"

namespace nova::renderer::renderpack {
    using namespace spirv_cross;

    RX_LOG("RenderpackConvert", logger);

    ShaderSource to_shader_source(const RenderpackShaderSource& rp_source) {
        ShaderSource source{};

        source.filename = rp_source.filename;
        source.source = rp_source.source;

        return source;
    }

    rhi::VertexFieldFormat to_rhi_vertex_format(const SPIRType& spirv_type) {
        switch(spirv_type.basetype) {
            case SPIRType::UInt:
                return rhi::VertexFieldFormat::Uint;

            case SPIRType::Float: {
                switch(spirv_type.vecsize) {
                    case 2:
                        return rhi::VertexFieldFormat::Float2;

                    case 3:
                        return rhi::VertexFieldFormat::Float3;

                    case 4:
                        return rhi::VertexFieldFormat::Float4;

                    default:
                        logger->error("Nova does not support float fields with %u vector elements", spirv_type.vecsize);
                        return rhi::VertexFieldFormat::Invalid;
                }
            };

            case SPIRType::Unknown:
                [[fallthrough]];
            case SPIRType::Void:
                [[fallthrough]];
            case SPIRType::Boolean:
                [[fallthrough]];
            case SPIRType::SByte:
                [[fallthrough]];
            case SPIRType::UByte:
                [[fallthrough]];
            case SPIRType::Short:
                [[fallthrough]];
            case SPIRType::UShort:
                [[fallthrough]];
            case SPIRType::Int:
                [[fallthrough]];
            case SPIRType::Int64:
                [[fallthrough]];
            case SPIRType::UInt64:
                [[fallthrough]];
            case SPIRType::AtomicCounter:
                [[fallthrough]];
            case SPIRType::Half:
                [[fallthrough]];
            case SPIRType::Double:
                [[fallthrough]];
            case SPIRType::Struct:
                [[fallthrough]];
            case SPIRType::Image:
                [[fallthrough]];
            case SPIRType::SampledImage:
                [[fallthrough]];
            case SPIRType::Sampler:
                [[fallthrough]];
            case SPIRType::AccelerationStructureNV:
                [[fallthrough]];
            case SPIRType::ControlPointArray:
                [[fallthrough]];
            case SPIRType::Char:
                [[fallthrough]];
            default:
                logger->error("Nova does not support vertex fields of type %u", spirv_type.basetype);
        }

        return {};
    }

    std::vector<rhi::RhiVertexField> get_vertex_fields(const ShaderSource& vertex_shader) {
        // TODO: Figure out if there's a better way to reflect on the shader
        const CompilerGLSL shader_compiler{vertex_shader.source.data(), vertex_shader.source.size()};

        const auto& shader_vertex_fields = shader_compiler.get_shader_resources().stage_inputs;

        std::vector<rhi::RhiVertexField> vertex_fields;
        vertex_fields.reserve(shader_vertex_fields.size());

        for(const auto& spirv_field : shader_vertex_fields) {
            const auto& spirv_type = shader_compiler.get_type(spirv_field.base_type_id);
            const auto format = to_rhi_vertex_format(spirv_type);

            vertex_fields.emplace_back(spirv_field.name.c_str(), format);
        }

        return vertex_fields;
    }

    PrimitiveTopology to_primitive_topology(const RPPrimitiveTopology primitive_mode) {
        switch(primitive_mode) {
            case RPPrimitiveTopology::Triangles:
                return PrimitiveTopology::TriangleList;

            case RPPrimitiveTopology::Lines:
                return PrimitiveTopology::LineList;

            default:
                return PrimitiveTopology::TriangleList;
        }
    }

    CompareOp to_compare_op(const RPCompareOp compare_op) {
        switch(compare_op) {
            case RPCompareOp::Never:
                return CompareOp::Never;

            case RPCompareOp::Less:
                return CompareOp::Less;

            case RPCompareOp::LessEqual:
                return CompareOp::LessEqual;

            case RPCompareOp::Greater:
                return CompareOp::Greater;

            case RPCompareOp::GreaterEqual:
                return CompareOp::GreaterEqual;

            case RPCompareOp::Equal:
                return CompareOp::Equal;

            case RPCompareOp::NotEqual:
                return CompareOp::NotEqual;

            case RPCompareOp::Always:
                return CompareOp::Always;

            default:
                return CompareOp::Greater;
        }
    }

    StencilOp to_stencil_op(const RPStencilOp stencil_op) {
        switch(stencil_op) {
            case RPStencilOp::Keep:
                return StencilOp::Keep;

            case RPStencilOp::Zero:
                return StencilOp::Zero;

            case RPStencilOp::Replace:
                return StencilOp::Replace;

            case RPStencilOp::Increment:
                return StencilOp::Increment;

            case RPStencilOp::IncrementAndWrap:
                return StencilOp::IncrementAndWrap;

            case RPStencilOp::Decrement:
                return StencilOp::Decrement;

            case RPStencilOp::DecrementAndWrap:
                return StencilOp::DecrementAndWrap;

            case RPStencilOp::Invert:
                return StencilOp::Invert;

            default:
                return StencilOp::Keep;
        }
    }

    BlendFactor to_blend_factor(const RPBlendFactor blend_factor) {
        switch(blend_factor) {
            case RPBlendFactor::One:
                return BlendFactor::One;

            case RPBlendFactor::Zero:
                return BlendFactor::Zero;

            case RPBlendFactor::SrcColor:
                return BlendFactor::SrcColor;

            case RPBlendFactor::DstColor:
                return BlendFactor::DstColor;

            case RPBlendFactor::OneMinusSrcColor:
                return BlendFactor::OneMinusSrcColor;

            case RPBlendFactor::OneMinusDstColor:
                return BlendFactor::OneMinusDstColor;

            case RPBlendFactor::SrcAlpha:
                return BlendFactor::SrcAlpha;

            case RPBlendFactor::DstAlpha:
                return BlendFactor::DstAlpha;

            case RPBlendFactor::OneMinusSrcAlpha:
                return BlendFactor::OneMinusSrcAlpha;

            case RPBlendFactor::OneMinusDstAlpha:
                return BlendFactor::OneMinusDstAlpha;

            default:
                return BlendFactor::One;
        }
    }

    rx::optional<RhiGraphicsPipelineState> to_pipeline_state_create_info(const PipelineData& data, const Rendergraph& rendergraph) {
        constexpr auto npos = std::vector<RasterizerState>::k_npos;

        RhiGraphicsPipelineState info{};

        info.name = data.name;

        // Shaders
        info.vertex_shader = to_shader_source(data.vertex_shader);
        if(data.geometry_shader) {
            info.geometry_shader = to_shader_source(*data.geometry_shader);
        }

        if(data.fragment_shader) {
            info.pixel_shader = to_shader_source(*data.fragment_shader);
        }

        info.vertex_fields = get_vertex_fields(info.vertex_shader);

        // Viewport and scissor test
        const auto* pass = rendergraph.get_renderpass(data.pass);
        if(pass == nullptr) {
            logger->error("Could not find render pass %s, which pipeline %s needs", data.pass, data.name);
            return rx::nullopt;
        }
        info.viewport_size = pass->framebuffer->size;

        info.enable_scissor_test = data.scissor_mode == ScissorTestMode::DynamicScissorRect;

        // Input assembly
        info.topology = to_primitive_topology(data.primitive_mode);

        // Rasterizer state
        if(data.states.find(RasterizerState::InvertCulling) != npos) {
            info.rasterizer_state.cull_mode = PrimitiveCullingMode::FrontFace;

        } else if(data.states.find(RasterizerState::DisableCulling) != npos) {
            info.rasterizer_state.cull_mode = PrimitiveCullingMode::None;
        }

        info.rasterizer_state.depth_bias = data.depth_bias;
        info.rasterizer_state.slope_scaled_depth_bias = data.slope_scaled_depth_bias;

        // Depth state
        if(data.states.find(RasterizerState::DisableDepthTest) != npos) {
            info.depth_state = rx::nullopt;

        } else {
            if(data.states.find(RasterizerState::DisableDepthWrite) != npos) {
                info.depth_state->enable_depth_write = false;

                info.depth_state->compare_op = to_compare_op(data.depth_func);
            }
        }

        // Stencil state
        if(data.states.find(RasterizerState::EnableStencilTest) != npos) {
            info.stencil_state = StencilState{};
            if(data.front_face) {
                info.stencil_state->front_face_op.fail_op = to_stencil_op(data.front_face->fail_op);
                info.stencil_state->front_face_op.pass_op = to_stencil_op(data.front_face->fail_op);
                info.stencil_state->front_face_op.depth_fail_op = to_stencil_op(data.front_face->depth_fail_op);
                info.stencil_state->front_face_op.compare_op = to_compare_op(data.front_face->compare_op);
                info.stencil_state->front_face_op.compare_mask = data.front_face->compare_mask;
                info.stencil_state->front_face_op.write_mask = data.front_face->write_mask;
                info.stencil_state->front_face_op.reference_value = data.stencil_ref;
            }
            if(data.back_face) {
                info.stencil_state->back_face_op.fail_op = to_stencil_op(data.back_face->fail_op);
                info.stencil_state->back_face_op.pass_op = to_stencil_op(data.back_face->fail_op);
                info.stencil_state->back_face_op.depth_fail_op = to_stencil_op(data.back_face->depth_fail_op);
                info.stencil_state->back_face_op.compare_op = to_compare_op(data.back_face->compare_op);
                info.stencil_state->back_face_op.compare_mask = data.back_face->compare_mask;
                info.stencil_state->back_face_op.write_mask = data.back_face->write_mask;
                info.stencil_state->back_face_op.reference_value = data.stencil_ref;
            }
        }

        // Blend state
        if(data.states.find(RasterizerState::Blending) != npos) {
            info.blend_state = BlendState{};
            info.blend_state->render_target_states.resize(pass->framebuffer->num_attachments);
            info.blend_state->render_target_states.each_fwd([&](RenderTargetBlendState& target_blend) {
                target_blend.enable = true;
                target_blend.src_color_factor = to_blend_factor(data.source_color_blend_factor);
                target_blend.dst_color_factor = to_blend_factor(data.destination_color_blend_factor);
                target_blend.color_op = BlendOp::Add;

                target_blend.src_alpha_factor = to_blend_factor(data.source_alpha_blend_factor);
                target_blend.dst_alpha_factor = to_blend_factor(data.destination_alpha_blend_factor);
                target_blend.alpha_op = BlendOp::Add;
            });
        }

        if(data.states.find(RasterizerState::DisableColorWrite) != npos) {
            info.enable_color_write = false;
        }

        if(data.states.find(RasterizerState::DisableAlphaWrite) != npos) {
            info.enable_alpha_write = false;
        }

        const auto& pass_data = rendergraph.get_metadata_for_renderpass(data.pass);
        if(!pass_data) {
            logger->error("Could not retrieve metadata for renderpass %s. Why can we retrieve the renderpass but not its metadata?",
                          data.pass);
            return rx::nullopt;
        }

        info.color_attachments = pass_data->data.texture_outputs;
        info.depth_texture = pass_data->data.depth_texture;

        return info;
    }
}; // namespace nova::renderer::renderpack
