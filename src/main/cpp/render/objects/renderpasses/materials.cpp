/*!
 * \author ddubois 
 * \date 06-Nov-17.
 */

#include "materials.h"
#include <easylogging++.h>
#include "../../../utils/utils.h"

namespace nova {
    material_state create_material_from_json(const std::string& material_state_name, const std::string& parent_state_name, const nlohmann::json& material_json) {
        material_state ret_val = {};

        ret_val.name = material_state_name;
        ret_val.parent = parent_state_name;

        if_contains_key(material_json, "states", [&](const auto& states) {
            for(auto& state : states) {
                state_enum decoded_state = decode_state(state);
                ret_val.states.push_back(decoded_state);
            }
        });

        if_contains_key(material_json, "defines", [&](const auto& defines) {
            for(auto& define : defines) {
                ret_val.defines.push_back(define);
            }
        });

        if_contains_key(material_json, "samplerStates", [&](const auto& sampler_states) {
            for(auto& sampler_state : sampler_states) {
                ret_val.sampler_states.push_back(decode_sampler_state(sampler_state));
            }
        });

        if_contains_key(material_json, "depthBias", [&](const auto& depth_bias) {
            ret_val.depth_bias = depth_bias;
        });

        if_contains_key(material_json, "slopeScaledDepthBias", [&](const auto& slope_scaled_depth_bias){
            ret_val.slope_scaled_depth_bias = slope_scaled_depth_bias;
        });

        if_contains_key(material_json, "depthBiasOGL", [&](const auto& depth_bias_ogl) {
            ret_val.depth_bias = depth_bias_ogl;
        });

        if_contains_key(material_json, "slopeScaledDepthBiasOGL", [&](const auto& slope_scaled_depth_bias_ogl){
            ret_val.slope_scaled_depth_bias = slope_scaled_depth_bias_ogl;
        });

        if_contains_key(material_json, "vertexShader", [&](const auto& vertex_shader) {
            ret_val.vertex_shader = vertex_shader;
        });

        if_contains_key(material_json, "fragmentShader", [&](const auto& fragment_shader) {
            ret_val.fragment_shader = fragment_shader;
        });

        if_contains_key(material_json, "geometryShader", [&](const nlohmann::json& geometry_shader) {
            ret_val.geometry_shader = std::experimental::make_optional(geometry_shader.get<std::string>());
        });

        if_contains_key(material_json, "tessellationEvaluationShader", [&](const nlohmann::json& tese_shader) {
            ret_val.tessellation_evaluation_shader = std::experimental::make_optional(tese_shader.get<std::string>());
        });

        if_contains_key(material_json, "tessellationControlShader", [&](const nlohmann::json& tesc_shader) {
            ret_val.tessellation_control_shader = std::experimental::make_optional(tesc_shader.get<std::string>());
        });

        if_contains_key(material_json, "vertexFields", [&](const nlohmann::json& vertex_fields) {
            for(const auto& vertex_field : vertex_fields) {
                ret_val.vertex_fields.push_back(decode_vertex_field(vertex_field));
            }
        });

        if_contains_key(material_json, "frontFace", [&](const nlohmann::json& front_face_obj) {
            ret_val.front_face = decode_stencil_buffer_state(front_face_obj);
        });

        if_contains_key(material_json, "backFace", [&](const nlohmann::json back_face_obj) {
            ret_val.back_face = decode_stencil_buffer_state(back_face_obj);
        });

        return ret_val;
    }

    state_enum decode_state(const std::string& state_to_decode) {
        if(state_to_decode == "Blending") {
            return state_enum::blending;

        } else if(state_to_decode == "InvertCulling") {
            return state_enum::invert_culing;

        } else if(state_to_decode == "DisableCulling") {
            return state_enum::disable_culling;

        } else if(state_to_decode == "DisableDepthWrite") {
            return state_enum::disable_depth_write;

        } else if(state_to_decode == "EnableStencilTest") {
            return state_enum::enable_stencil_test;

        } else if(state_to_decode == "StencilWrite") {
            return state_enum::stencil_write;

        } else if(state_to_decode == "DisableColorWrite") {
            return state_enum::disable_color_write;

        } else if(state_to_decode == "EnableAlphaToCoverage") {
            return state_enum::enable_alpha_to_coverage;
        }

        // EVERYTHING IS A FATAL ERROR WHOO
        // TODO: Fail gracefully
        LOG(FATAL) << "Invalid value in the states array: '" << state_to_decode << "'";
    }

    sampler_state decode_sampler_state(const nlohmann::json& json) {
        sampler_state new_sampler_state = {};

        new_sampler_state.sampler_index = json["samplerIndex"];
        new_sampler_state.filter = decode_texture_filter_enum(json["filter"]);
        new_sampler_state.wrap_mode = decode_wrap_mode_enum(json["wrapMode"]);

        return new_sampler_state;
    }

    wrap_mode_enum decode_wrap_mode_enum(const std::string &wrap_mode) {
        if(wrap_mode == "Clamp") {
            return wrap_mode_enum::clamp;

        } else if(wrap_mode == "Repeat") {
            return wrap_mode_enum::repeat;
        }

        LOG(FATAL) << "Invalid wrap mode: '" << wrap_mode << "'";
    }

    texture_filter_enum decode_texture_filter_enum(const std::string &texture_filter_enum_str) {
        if(texture_filter_enum_str == "TexelAA") {
            return texture_filter_enum::texel_aa;

        } else if(texture_filter_enum_str == "Bilinear") {
            return texture_filter_enum ::bilinear;

        } else if(texture_filter_enum_str == "Point") {
            return texture_filter_enum::point;
        }

        LOG(FATAL) << "Invalid value for a texture filter enum: '" << texture_filter_enum_str << "'";
    }

    vertex_field_enum decode_vertex_field(const nlohmann::json &vertex_field_json) {
        const std::string vertex_field_str = vertex_field_json["field"];

        if(vertex_field_str == "Position") {
            return vertex_field_enum::position;

        } else if(vertex_field_str == "Color") {
            return vertex_field_enum::color;

        } else if(vertex_field_str == "UV0" || vertex_field_str == "MainUV") {
            return vertex_field_enum::main_uv;

        } else if(vertex_field_str == "UV1" || vertex_field_str == "LightmapUV") {
            return vertex_field_enum::lightmap_uv;

        } else if(vertex_field_str == "Normal") {
            return vertex_field_enum::normal;

        } else if(vertex_field_str == "Tangent") {
            return vertex_field_enum::tangent;

        } else if(vertex_field_str == "MidTexCoord") {
            return vertex_field_enum::mid_tex_coord;

        } else if(vertex_field_str == "VirtualTextureId") {
            return vertex_field_enum::virtual_texture_id;

        } else if(vertex_field_str == "McEntityId") {
            return vertex_field_enum::mc_entity_id;

        } else if(vertex_field_str == "Empty") {
            return vertex_field_enum::empty;

        }

        LOG(FATAL) << "Invalid vertex field: '" << vertex_field_str << "'";
    }

    comparison_func_enum decode_comparison_func_enum(const std::string& comparison_func) {
        if(comparison_func == "Always") {
            return comparison_func_enum::always;

        } else if(comparison_func == "Never") {
            return comparison_func_enum::never;

        } else if(comparison_func == "Less") {
            return comparison_func_enum::less;

        } else if(comparison_func == "LessEqual") {
            return comparison_func_enum::less_equal;

        } else if(comparison_func == "GreaterEqual") {
            return comparison_func_enum::greater_equal;

        } else if(comparison_func == "Equal") {
            return comparison_func_enum::equal;

        } else if(comparison_func == "Replace") {
            return comparison_func_enum::replace;

        } else if(comparison_func == "NotEqual") {
            return comparison_func_enum::not_equal;
        }

        LOG(FATAL) << "Invalid comparison function '" << comparison_func << "'";
    }

    stencil_or_depth_op_enum decode_stencil_or_depth_op_enum(const std::string& op) {
        if(op == "Keep") {
            return stencil_or_depth_op_enum::keep;

        } else if(op == "Zero") {
            return stencil_or_depth_op_enum::zero;

        } else if(op == "Replace") {
            return stencil_or_depth_op_enum::replace;

        } else if(op == "Increment") {
            return stencil_or_depth_op_enum::increment;

        } else if(op == "IncrementAndWrap") {
            return stencil_or_depth_op_enum::increment_and_wrap;

        } else if(op == "Decrement") {
            return stencil_or_depth_op_enum::decrement;

        } else if(op == "DecrementAndSwap") {
            return stencil_or_depth_op_enum::decrement_and_wrap;

        } else if(op == "Invert") {
            return stencil_or_depth_op_enum::invert;

        }

        LOG(FATAL) << "Invalid stencil or depth operation '" << op << "'";
    }

    stencil_buffer_state decode_stencil_buffer_state(const nlohmann::json &json) {
        auto ret_val = stencil_buffer_state{};

        if_contains_key(json, "stencilFunc", [&](const std::string& stencil_func) {
            ret_val.stencil_func = decode_comparison_func_enum(stencil_func);
        });

        if_contains_key(json, "stencilFailOp", [&](const std::string& stencil_fail_op) {
            ret_val.stencil_fail_op = decode_stencil_or_depth_op_enum(stencil_fail_op);
        });

        if_contains_key(json, "stencilDepthFailOp", [&](const std::string& stencil_depth_fail_op) {
            ret_val.stencil_depth_fail_op = decode_stencil_or_depth_op_enum(stencil_depth_fail_op);
        });

        if_contains_key(json, "stencilPassOp", [&](const std::string& stencil_pass_op) {
            ret_val.stencil_pass_op = decode_stencil_or_depth_op_enum(stencil_pass_op);
        });

        return ret_val;
    }
}
