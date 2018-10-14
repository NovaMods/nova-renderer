/*!
 * \author ddubois
 * \date 07-Sep-18.
 */

#include "json_interop.hpp"
#include "../json_utils.hpp"

namespace nova {
    void from_json(const nlohmann::json &j, texture_format &format) {
        format.pixel_format = get_json_value<pixel_format_enum>(j, "format", pixel_format_enum::RGB8, pixel_format_enum_from_string);
        format.dimension_type = get_json_value<texture_dimension_type_enum>(j, "dimensionType", texture_dimension_type_enum::ScreenRelative, texture_dimension_type_enum_from_string);
        format.width = get_json_value<float>(j, "width").value_or(0);
        format.height = get_json_value<float>(j, "height").value_or(0);
    }

    void from_json(const nlohmann::json &j, texture_resource_data &tex) {
        tex.name = get_json_value<std::string>(j, "name").value();
        tex.format = get_json_value<texture_format>(j, "format").value();
    }

    void from_json(const nlohmann::json &j, sampler_state_data &sampler) {
        sampler.filter = get_json_value<texture_filter_enum>(j, "filter", texture_filter_enum::Point, texture_filter_enum_from_string);
        sampler.wrap_mode = get_json_value<wrap_mode_enum>(j, "wrapMode", wrap_mode_enum::Clamp, wrap_mode_enum_from_string);
    }

    void from_json(const nlohmann::json &j, shaderpack_resources_data &res) {
        res.textures = get_json_array<texture_resource_data>(j, "textures");
        res.samplers = get_json_array<sampler_state_data>(j, "samplers");
    }

    void from_json(const nlohmann::json &j, render_pass_data &pass) {
        pass.dependencies = get_json_array<std::string>(j, "dependencies");
        pass.texture_inputs = get_json_value<input_textures>(j, "textureInputs", input_textures{});
        pass.texture_outputs = get_json_array<texture_attachment>(j, "textureOutputs");
        pass.depth_texture = get_json_value<texture_attachment>(j, "depthTexture", texture_attachment{});
        pass.name = get_json_value<std::string>(j, "name", "<NAME_MISSING>");
    }

    void from_json(const nlohmann::json &j, input_textures &inputs) {
        inputs.bound_textures = get_json_array<std::string>(j, "boundTextures");
        inputs.color_attachments = get_json_array<std::string>(j, "colorAttachments");
    }

    void from_json(const nlohmann::json &j, stencil_op_state &stencil_op) {
        stencil_op.fail_op = get_json_value<stencil_op_enum>(j, "failOp", stencil_op_enum::Keep, stencil_op_enum_from_string);
        stencil_op.pass_op = get_json_value<stencil_op_enum>(j, "passOp", stencil_op_enum::Keep, stencil_op_enum_from_string);
        stencil_op.depth_fail_op = get_json_value<stencil_op_enum>(j, "depthFailOp", stencil_op_enum::Keep, stencil_op_enum_from_string);
        stencil_op.compare_op = get_json_value<compare_op_enum>(j, "compareOp", compare_op_enum::Equal, compare_op_enum_from_string);
        stencil_op.compare_mask = get_json_value<uint32_t>(j, "compareMask", 0);
        stencil_op.write_mask = get_json_value<uint32_t>(j, "writeMask", 0);
    }

    void from_json(const nlohmann::json& j, vertex_field_data& vertex_data) {
        vertex_data.semantic_name = get_json_value<std::string>(j, "name").value();
        vertex_data.field = get_json_value<vertex_field_enum>(j, "field", vertex_field_enum_from_string).value();
    }

    void from_json(const nlohmann::json &j, pipeline_data &pipeline) {
        pipeline.name = get_json_value<std::string>(j, "name").value();
        pipeline.parent_name = get_json_value<std::string>(j, "parent").value_or("");
        pipeline.pass = get_json_value<std::string>(j, "pass").value();
        pipeline.defines = get_json_array<std::string>(j, "defines");
        pipeline.states = get_json_array<state_enum>(j, "states", state_enum_from_string);
        pipeline.vertex_fields = get_json_array<vertex_field_data>(j, "vertexFields");
        pipeline.front_face = get_json_value<stencil_op_state>(j, "frontFace");
        pipeline.back_face = get_json_value<stencil_op_state>(j, "backFace");
        pipeline.fallback = get_json_value<std::string>(j, "fallback").value_or("");
        pipeline.depth_bias = get_json_value<float>(j, "depthBias", 0);
        pipeline.slope_scaled_depth_bias = get_json_value<float>(j, "slopeScaledDepthBias", 0);
        pipeline.stencil_ref = get_json_value<uint32_t>(j, "stencilRef", 0);
        pipeline.stencil_read_mask = get_json_value<uint32_t>(j, "stencilReadMask", 0);
        pipeline.stencil_write_mask = get_json_value<uint32_t>(j, "stencilWriteMask", 0);
        pipeline.msaa_support = get_json_value<msaa_support_enum>(j, "msaaSupport", msaa_support_enum::None, msaa_support_enum_from_string);
        pipeline.primitive_mode = get_json_value<primitive_topology_enum>(j, "primitiveMode", primitive_topology_enum::Triangles, primitive_topology_enum_from_string);
        pipeline.source_blend_factor = get_json_value<blend_factor_enum>(j, "sourceBlendFactor", blend_factor_enum::One, blend_factor_enum_from_string);
        pipeline.destination_blend_factor = get_json_value<blend_factor_enum>(j, "destBlendFactor", blend_factor_enum::Zero, blend_factor_enum_from_string);
        pipeline.alpha_src = get_json_value<blend_factor_enum>(j, "alphaSrc", blend_factor_enum::One, blend_factor_enum_from_string);
        pipeline.alpha_dst = get_json_value<blend_factor_enum>(j, "alphaDest", blend_factor_enum::Zero, blend_factor_enum_from_string);
        pipeline.depth_func = get_json_value<compare_op_enum>(j, "depthFunc", compare_op_enum::Less, compare_op_enum_from_string);
        pipeline.render_queue = get_json_value<render_queue_enum>(j, "renderQueue", render_queue_enum::Opaque, render_queue_enum_from_string);


        pipeline.vertex_shader.filename = get_json_value<std::string>(j, "vertexShader", "<SHADER_MISSING>");

        std::optional<std::string> geometry_shader_name = get_json_value<std::string>(j, "geometryShader");
        if(geometry_shader_name) {
            pipeline.geometry_shader = std::make_optional<shader_source>();
            (*pipeline.geometry_shader).filename = *geometry_shader_name;
        }

        std::optional<std::string> tess_control_shader_name = get_json_value<std::string>(j, "tessellationControlShader");
        if(tess_control_shader_name) {
            pipeline.tessellation_control_shader = std::make_optional<shader_source>();
            (*pipeline.tessellation_control_shader).filename = *tess_control_shader_name;
        }

        std::optional<std::string> tess_eval_shader_name = get_json_value<std::string>(j, "tessellationEvalShader");
        if(tess_eval_shader_name) {
            pipeline.tessellation_evaluation_shader = std::make_optional<shader_source>();
            (*pipeline.tessellation_evaluation_shader).filename = *tess_eval_shader_name;
        }

        std::optional<std::string> fragment_shader_name = get_json_value<std::string>(j, "fragmentShader");
        if(fragment_shader_name) {
            pipeline.fragment_shader = std::make_optional<shader_source>();
            (*pipeline.fragment_shader).filename = *geometry_shader_name;
        }
    }

    void from_json(const nlohmann::json &j, material_pass &pass) {
        pass.pipeline = get_json_value<std::string>(j, "pipeline").value();
        pass.bindings = get_json_value<std::unordered_map<std::string, std::string>>(j, "bindings").value();
    }

    void from_json(const nlohmann::json &j, material_data &mat) {
        mat.passes = get_json_array<material_pass>(j, "passes");
        mat.geometry_filter = get_json_value<std::string>(j, "filter").value();
    }

    void from_json(const nlohmann::json &j, texture_attachment &tex) {
        tex.name = get_json_value<std::string>(j, "name").value();
        tex.clear = get_json_value<bool>(j, "clear", false);
    }

    void from_json(const nlohmann::json &j, std::vector<render_pass_data> &passes) {
        for(const auto &node : j) {
            passes.push_back(node.get<render_pass_data>());
        }
    }
}  // namespace nova