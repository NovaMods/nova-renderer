/*!
 * \author ddubois 
 * \date 06-Nov-17.
 */

#include "materials.h"
#include <easylogging++.h>
#include "../../../utils/utils.h"

namespace nova {
    material_state create_material_from_json(const std::string& material_state_name, const optional<std::string>& parent_state_name, const nlohmann::json& material_json) {
        material_state ret_val = {};

        ret_val.name = material_state_name;
        ret_val.parent_name = parent_state_name;

        ret_val.states = get_json_value<std::vector<state_enum>>(material_json, "states", [&](const auto& states) {
            auto vec = std::vector<state_enum>{};
            for(auto& state : states) {
                vec.push_back(state_enum::from_string(state));
            }
            return vec;
        });

        ret_val.defines = get_json_value<std::vector<std::string>>(material_json, "defines", [&](const auto& defines) {
            auto vec = std::vector<std::string>{};
            for(auto& define : defines) {
                vec.push_back(define);
            }
            return vec;
        });

        ret_val.sampler_states = get_json_value<std::vector<sampler_state>>(material_json, "samplerStates", [&](const auto& sampler_states) {
            auto vec = std::vector<sampler_state>{};
            for(auto& sampler_state : sampler_states) {
                vec.push_back(decode_sampler_state(sampler_state));
            }
            return vec;
        });

        // Sometimes I have to use the type name, sometimes auto works. Go figure
        ret_val.depth_bias = get_json_value<float>(material_json, "depthBias");

        ret_val.slope_scaled_depth_bias = get_json_value<float>(material_json, "slopeScaledDepthBias");

        ret_val.depth_bias = get_json_value<float>(material_json, "depthBiasOGL");

        ret_val.slope_scaled_depth_bias = get_json_value<float>(material_json, "slopeScaledDepthBiasOGL");

        ret_val.vertex_shader = get_json_value<std::string>(material_json, "vertexShader");

        ret_val.fragment_shader = get_json_value<std::string>(material_json, "fragmentShader");

        ret_val.geometry_shader  = get_json_value<std::string>(material_json, "geometryShader");

        ret_val.tessellation_evaluation_shader = get_json_value<std::string>(material_json, "tessellationEvaluationShader");

        ret_val.tessellation_control_shader = get_json_value<std::string>(material_json, "tessellationControlShader");

        ret_val.vertex_fields = get_json_value<std::vector<vertex_field_enum>>(material_json, "vertexFields", [&](const nlohmann::json& vertex_fields) {
            auto vec = std::vector<vertex_field_enum>{};
            for(const auto& vertex_field : vertex_fields) {
                vec.push_back(vertex_field_enum::from_string(vertex_field["field"]));
            }
            return vec;
        });

        ret_val.front_face = get_json_value<stencil_op_state>(material_json, "frontFace", decode_stencil_buffer_state);

        ret_val.back_face = get_json_value<stencil_op_state>(material_json, "backFace", decode_stencil_buffer_state);

        ret_val.stencil_ref = get_json_value<uint32_t>(material_json, "stencilRef");

        ret_val.stencil_read_mask = get_json_value<uint32_t>(material_json, "stencilReadMask");

        if(ret_val.stencil_write_mask) {
            if(ret_val.front_face) {
                (*ret_val.front_face).compare_mask = ret_val.stencil_write_mask;
            }
            if(ret_val.back_face) {
                (*ret_val.back_face).compare_mask = ret_val.stencil_write_mask;
            }
        }

        ret_val.stencil_write_mask = get_json_value<uint32_t>(material_json, "stencilWriteMask");

        if(ret_val.stencil_write_mask) {
            if(ret_val.front_face) {
                (*ret_val.front_face).write_mask = ret_val.stencil_write_mask;
            }
            if(ret_val.back_face) {
                (*ret_val.back_face).write_mask = ret_val.stencil_write_mask;
            }
        }

        ret_val.msaa_support = get_json_value<msaa_support_enum>(material_json, "msaaSupport", msaa_support_enum::from_string);

        ret_val.primitive_mode = get_json_value<vk::PrimitiveTopology>(material_json, "primitiveMode", decode_primitive_mode_enum);

        ret_val.source_blend_factor = get_json_value<vk::BlendFactor>(material_json, "blendSrc", decode_blend_source_enum);

        ret_val.destination_blend_factor = get_json_value<vk::BlendFactor>(material_json, "blendDst", decode_blend_source_enum);

        ret_val.textures = get_json_value<std::vector<texture>>(material_json, "textures", [&](const nlohmann::json& textures) {
            auto vec = std::vector<texture>{};

            for(const auto& texture_field : textures) {
                vec.push_back(decode_texture(texture_field));
            }

            return vec;
        });

        ret_val.alpha_src = get_json_value<vk::BlendFactor>(material_json, "alphaSrc", decode_blend_source_enum);
        ret_val.alpha_dst = get_json_value<vk::BlendFactor>(material_json, "alphaDst", decode_blend_source_enum);
        ret_val.depth_func = get_json_value<vk::CompareOp>(material_json, "depthFunc", decode_comparison_func_enum);

        ret_val.filters = get_json_value<std::string>(material_json, "filters");
        ret_val.fallback = get_json_value<std::string>(material_json, "fallback");
        ret_val.pass_index = get_json_value<uint32_t>(material_json, "passIndex");
        ret_val.has_transparency = get_json_value<bool>(material_json, "hasTransparency");
        ret_val.has_cutout = get_json_value<bool>(material_json, "hasCutout");

        ret_val.outputs = get_json_value<std::vector<output_info>>(material_json, "outputs", [&](const nlohmann::json outputs){
            auto vec = std::vector<output_info>{};
            for(const auto& output_field : outputs) {
                vec.push_back(decode_outputs(output_field));
            }
            return vec;
        });

        ret_val.pass = get_json_value<pass_enum>(material_json, "pass", pass_enum::from_string);

        return ret_val;
    }

    sampler_state decode_sampler_state(const nlohmann::json& json) {
        sampler_state new_sampler_state = {};

        new_sampler_state.sampler_index = get_json_value<std::uint32_t>(json, "samplerIndex").value_or(0);
        new_sampler_state.filter = get_json_value<texture_filter_enum>(json, "textureFilter", texture_filter_enum::from_string).value_or(texture_filter_enum::Point);
        new_sampler_state.wrap_mode = get_json_value<wrap_mode_enum>(json, "wrapMode", wrap_mode_enum::from_string).value_or(wrap_mode_enum::Clamp);

        return new_sampler_state;
    }

    vk::CompareOp decode_comparison_func_enum(const std::string& comparison_func) {
        if(comparison_func == "Always") {
            return vk::CompareOp::eAlways;

        } else if(comparison_func == "Never") {
            return vk::CompareOp::eNever;

        } else if(comparison_func == "Less") {
            return vk::CompareOp::eLess;

        } else if(comparison_func == "LessEqual") {
            return vk::CompareOp::eLessOrEqual;

        } else if(comparison_func == "GreaterEqual") {
            return vk::CompareOp::eGreaterOrEqual;

        } else if(comparison_func == "Equal") {
            return vk::CompareOp::eEqual;

        } else if(comparison_func == "NotEqual") {
            return vk::CompareOp::eNotEqual;
        }

        LOG(FATAL) << "Invalid comparison function '" << comparison_func << "'";
    }

    vk::StencilOp decode_stencil_op_enum(const std::string &op) {
        if(op == "Keep") {
            return vk::StencilOp::eKeep;

        } else if(op == "Zero") {
            return vk::StencilOp::eZero;

        } else if(op == "Replace") {
            return vk::StencilOp::eReplace;

        } else if(op == "Increment") {
            return vk::StencilOp::eIncrementAndClamp;

        } else if(op == "IncrementAndWrap") {
            return vk::StencilOp::eIncrementAndWrap;

        } else if(op == "Decrement") {
            return vk::StencilOp::eDecrementAndClamp;

        } else if(op == "DecrementAndSwap") {
            return vk::StencilOp::eDecrementAndWrap;

        } else if(op == "Invert") {
            return vk::StencilOp::eInvert;

        }

        LOG(FATAL) << "Invalid stencil or depth operation '" << op << "'";
    }

    stencil_op_state decode_stencil_buffer_state(const nlohmann::json &json) {
        auto ret_val = stencil_op_state{};

        ret_val.compare_op = get_json_value<vk::CompareOp>(json, "stencilFunc", decode_comparison_func_enum);

        ret_val.fail_op = get_json_value<vk::StencilOp>(json, "stencilFailOp", decode_stencil_op_enum);

        ret_val.depth_fail_op = get_json_value<vk::StencilOp>(json, "stencilDepthFailOp", decode_stencil_op_enum);

        ret_val.pass_op = get_json_value<vk::StencilOp>(json, "stencilPassOp", decode_stencil_op_enum);

        return ret_val;
    }

    vk::PrimitiveTopology decode_primitive_mode_enum(const std::string& primitive_mode_str) {
        if(primitive_mode_str == "Line") {
            return vk::PrimitiveTopology::eLineList;

        } else if(primitive_mode_str == "Triangle") {
            return vk::PrimitiveTopology::eTriangleList;

        }

        LOG(FATAL) << "Invalid primitive mode: '" << primitive_mode_str << "'";
    }

    vk::BlendFactor decode_blend_source_enum(const std::string& blend_source_str) {
        if(blend_source_str == "SourceColor") {
            return vk::BlendFactor::eSrcColor;

        } else if(blend_source_str == "Zero") {
            return vk::BlendFactor::eZero;

        } else if(blend_source_str == "One") {
            return vk::BlendFactor::eOne;

        } else if(blend_source_str == "SourceAlpha") {
            return vk::BlendFactor::eSrcAlpha;

        } else if(blend_source_str == "OneMinusSrcAlpha") {
            return vk::BlendFactor::eOneMinusSrcAlpha;

        } else if(blend_source_str == "OneMinusSrcColor") {
            return vk::BlendFactor::eOneMinusSrcColor;

        } else if(blend_source_str == "DestColor") {
            return vk::BlendFactor::eDstColor;

        } else if(blend_source_str == "OneMinusDestColor") {
            return vk::BlendFactor::eOneMinusDstColor;

        }

        LOG(FATAL) << "Invalid blend source '" << blend_source_str << "'";
    }

    texture decode_texture(const nlohmann::json& texture_json) {
        auto tex = texture{};

        // If we don't have these, it's an error
        // TODO: A better parser for this stuff. Maybe extend optional to have an exception or error string or something?
        tex.index = get_json_value<uint32_t>(texture_json, "textureIndex").value();

        tex.texture_location = get_json_value<texture_location_enum>(texture_json, "textureLocation", texture_location_enum::from_string).value();

        tex.texture_name = get_json_value<std::string>(texture_json, "textureName").value();

        tex.calculate_mipmaps = get_json_value<bool>(texture_json, "calculateMipmaps");

        return tex;
    }

    output_info decode_outputs(const nlohmann::json& output_info_json) {
        auto ret_val = output_info{};

        ret_val.index = *get_json_value<uint8_t>(output_info_json, "index");
        ret_val.blending = *get_json_value<bool>(output_info_json, "blending");

        return ret_val;
    }

    vk::StencilOpState stencil_op_state::to_vk_stencil_op_state() const {
        auto ret_val = vk::StencilOpState{};

        ret_val.compareMask = compare_mask.value_or(0);
        ret_val.compareOp = compare_op.value_or(vk::CompareOp::eAlways);
        ret_val.depthFailOp = depth_fail_op.value_or(vk::StencilOp::eKeep);
        ret_val.failOp = fail_op.value_or(vk::StencilOp::eKeep);
        ret_val.passOp = pass_op.value_or(vk::StencilOp::eReplace);
        ret_val.writeMask = write_mask.value_or(0xFFFFFFFF);

        return ret_val;
    }
}
