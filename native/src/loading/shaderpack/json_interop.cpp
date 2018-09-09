/*!
 * \author ddubois 
 * \date 07-Sep-18.
 */

#include "json_interop.hpp"
#include "../utils.hpp"

namespace nova {
    void from_json(const nlohmann::json& j, texture_format& format) {
        format.pixel_format     = get_json_value<pixel_format_enum>(j, "format", pixel_format_enum::RGB8, pixel_format_enum::from_string);
        format.dimension_type   = get_json_value<texture_dimension_type_enum>(j, "dimensionType", texture_dimension_type_enum::from_string).value();
        format.width            = get_json_value<float>(j, "width").value_or(0);
        format.height           = get_json_value<float>(j, "height").value_or(0);
    }

    void from_json(const nlohmann::json& j, texture_resource& tex) {
        tex.name    = get_json_value<std::string>(j, "name").value_or("NAME_MISSING");
        tex.format  = get_json_value<texture_format>(j, "format").value();
    }

    void from_json(const nlohmann::json& j, sampler_state& sampler) {
        sampler.filter = get_json_value<texture_filter_enum>(j, "filter", texture_filter_enum::from_string);
        sampler.wrap_mode = get_json_value<wrap_mode_enum>(j, "wrapMode", wrap_mode_enum::from_string);
    }

    void from_json(const nlohmann::json& j, shaderpack_resources& res) {
        res.textures = get_json_array<texture_resource>(j, "textures");
        res.samplers = get_json_array<sampler_state>(j, "samplers");
    }

    void from_json(const nlohmann::json& j, render_pass& pass) {
        pass.dependencies       = get_json_array<std::string>(j, "dependencies");
        pass.texture_inputs     = get_json_value<input_textures>(j, "textureInputs");
        pass.texture_outputs    = get_json_array<texture_attachment>(j, "textureOutputs");
        pass.depth_texture      = get_json_value<texture_attachment>(j, "depthTexture");
    }

    void from_json(const nlohmann::json& j, input_textures& inputs) {
        inputs.bound_textures = get_json_array<std::string>(j, "boundTextures");
        inputs.color_attachments = get_json_array<std::string>(j, "colorAttachments");
    }

    void from_json(const nlohmann::json& j, stencil_op_state& stencil_op) {
        stencil_op.fail_op           = get_json_value<stencil_op_enum>(j, "failOp", stencil_op_enum::from_string);
        stencil_op.pass_op          = get_json_value<stencil_op_enum>(j, "passOp", stencil_op_enum::from_string);
        stencil_op.depth_fail_op    = get_json_value<stencil_op_enum>(j, "depthFailOp", stencil_op_enum::from_string);
        stencil_op.compare_op       = get_json_value<compare_op_enum>(j, "compareOp", compare_op_enum::from_string);
        stencil_op.compare_mask     = get_json_value<uint32_t>(j, "compareMask");
        stencil_op.write_mask       = get_json_value<uint32_t>(j, "writeMask");
    }

    void from_json(const nlohmann::json& j, pipeline_data& pipeline) {
        pipeline.parent_name                = get_json_value<std::string>(j, "parent");
        pipeline.pass                       = get_json_value<std::string>(j, "pass");
        pipeline.defines                    = get_json_array<std::string>(j, "defines");
        pipeline.states                     = get_json_array<state_enum>(j, "states", state_enum::from_string);
        pipeline.vertex_fields              = get_json_array<vertex_field_enum>(j, "vertexFields", vertex_field_enum::from_string);
        pipeline.front_face                 = get_json_value<stencil_op_state>(j, "frontFace");
        pipeline.back_face                  = get_json_value<stencil_op_state>(j, "backFace");
        pipeline.fallback                   = get_json_value<std::string>(j, "fallback");
        pipeline.depth_bias                 = get_json_value<float>(j, "depthBias");
        pipeline.slope_scaled_depth_bias    = get_json_value<float>(j, "slopeScaledDepthBias");
        pipeline.stencil_ref                = get_json_value<uint32_t>(j, "stencilRef");
        pipeline.stencil_read_mask          = get_json_value<uint32_t>(j, "stencilReadMask");
        pipeline.stencil_write_mask         = get_json_value<uint32_t>(j, "stencilWriteMask");
        pipeline.msaa_support               = get_json_value<msaa_support_enum>(j, "msaaSupport", msaa_support_enum::from_string);
        pipeline.primitive_mode             = get_json_value<primitive_topology_enum>(j, "primitiveMode", primitive_topology_enum::from_string);
        pipeline.source_blend_factor        = get_json_value<blend_factor_enum>(j, "sourceBlendFactor", blend_factor_enum::from_string);
        pipeline.destination_blend_factor   = get_json_value<blend_factor_enum>(j, "destBlendFactor", blend_factor_enum::from_string);
        pipeline.alpha_src                  = get_json_value<blend_factor_enum>(j, "alphaSrc", blend_factor_enum::from_string);
        pipeline.alpha_dst                  = get_json_value<blend_factor_enum>(j, "alphsDest", blend_factor_enum::from_string);
        pipeline.depth_func                 = get_json_value<compare_op_enum>(j, "depthFunc", compare_op_enum::from_string);
        pipeline.render_queue               = get_json_value<render_queue_enum>(j, "renderQueue", render_queue_enum::from_string);
    }

    void from_json(const nlohmann::json& j, material_pass& pass) {
        pass.pipeline = get_json_value<std::string>(j, "pipeline").value();
        pass.bindings = get_json_value<std::unordered_map<std::string, std::string>>(j, "bindings").value();
    }

    void from_json(const nlohmann::json& j, material_data& mat) {
        mat.name            = get_json_value<std::string>(j, "name").value();
        mat.passes          = get_json_array<material_pass>(j, "passes");
        mat.geometry_filter = get_json_value<std::string>(j, "filter").value();
    }

    void from_json(const nlohmann::json& j, texture_attachment& tex) {
        tex.name    = get_json_value<std::string>(j, "name").value_or("NAME_MISSING");
        tex.clear   = get_json_value<bool>(j, "clear").value_or(false);
    }

    void from_json(const nlohmann::json& j, std::vector<render_pass>& passes) {
        for(const auto& node : j) {
            passes.push_back(node.get<render_pass>());
        }
    }
}