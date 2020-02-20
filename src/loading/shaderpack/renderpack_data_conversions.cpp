#include "nova_renderer/renderpack_data_conversions.hpp"

#include "nova_renderer/shaderpack_data.hpp"

namespace nova::renderer::renderpack {
    ShaderSource to_shader_source(const shaderpack::RenderpackShaderSource& rp_source) {
        ShaderSource source{};

        source.filename = rp_source.filename;
        source.source = rp_source.source;

        return source;
    }

    PipelineStateCreateInfo to_pipeline_state_create_info(const shaderpack::PipelineData& data) {
        PipelineStateCreateInfo info{};

        info.name = data.name;
        info.vertex_shader = to_shader_source(data.vertex_shader);
        if(data.geometry_shader) {
            info.geometry_shader = to_shader_source(*data.geometry_shader);
        }

        if(data.fragment_shader) {
            info.pixel_shader = to_shader_source(*data.fragment_shader);
        }

        info.vertex_fields;

        return info;
    }
}; // namespace nova::renderer::renderpack
