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

        // TODO: Flesh out this function by converting to vertex_field_enum values
    }
}
