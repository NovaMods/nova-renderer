#include "nova_renderer/renderpack_data_conversions.hpp"

#include "nova_renderer/rhi/rhi_enums.hpp"
#include "nova_renderer/shaderpack_data.hpp"

#include "rx/core/log.h"
#include "spirv_glsl.hpp"
#include "nova_renderer/rendergraph.hpp"

namespace nova::renderer::renderpack {
    using namespace spirv_cross;

    RX_LOG("RenderpackConvert", logger);

    ShaderSource to_shader_source(const shaderpack::RenderpackShaderSource& rp_source) {
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
                        logger(rx::log::level::k_error, "Nova does not support float fields with %u vector elements", spirv_type.vecsize);
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
                logger(rx::log::level::k_error, "Nova does not support vertex fields of type %u", spirv_type.basetype);
        }

        return {};
    }

    rx::vector<rhi::VertexField> get_vertex_fields(const ShaderSource& vertex_shader) {
        const spirv_cross::CompilerGLSL shader_compiler{vertex_shader.source.data(), vertex_shader.source.size()};

        const auto& shader_vertex_fields = shader_compiler.get_shader_resources().stage_inputs;

        rx::vector<rhi::VertexField> vertex_fields;
        vertex_fields.reserve(shader_vertex_fields.size());

        for(const auto& spirv_field : shader_vertex_fields) {
            const auto& spirv_type = shader_compiler.get_type(spirv_field.base_type_id);
            const auto format = to_rhi_vertex_format(spirv_type);

            vertex_fields.emplace_back(spirv_field.name.c_str(), format);
        }

        return vertex_fields;
    }

    rx::optional<PipelineStateCreateInfo> to_pipeline_state_create_info(const shaderpack::PipelineData& data, const Rendergraph& rendergraph) {
        PipelineStateCreateInfo info{};

        info.name = data.name;
        info.vertex_shader = to_shader_source(data.vertex_shader);
        if(data.geometry_shader) {
            info.geometry_shader = to_shader_source(*data.geometry_shader);
        }

        if(data.fragment_shader) {
            info.pixel_shader = to_shader_source(*data.fragment_shader);
        }

        info.vertex_fields = get_vertex_fields(info.vertex_shader);
        
        const auto pass = rendergraph.get_renderpass(data.pass);
        if(pass == nullptr) {
            logger(rx::log::level::k_error, "Could not find render pass %s, which pipeline %s needs", data.pass, data.name);
            return rx::nullopt;
        }
        info.viewport_size = pass->framebuffer->size;

        return info;
    }
}; // namespace nova::renderer::renderpack
