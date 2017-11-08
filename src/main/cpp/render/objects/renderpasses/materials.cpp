/*!
 * \author ddubois 
 * \date 06-Nov-17.
 */

#include "materials.h"
#include <easylogging++.h>
#include "../../../utils/utils.h"

namespace nova {
    material_state create_from_json(const std::string& material_state_name, const std::string& parent_state_name, const nlohmann::json& material_json) {
        material_state ret_val = {};

        ret_val.name = material_state_name;
        ret_val.parent = parent_state_name;

        if_contains_key(material_json, "states", [&](nlohmann::json& states) {
            for(auto& state : states) {
                state_enum decoded_state = decode_state(state);
                ret_val.states.push_back(decoded_state);
            }
        });

        if_contains_key(material_json, "defines", [&](auto& defines) {
            for(auto& define : defines) {
                ret_val.defines.push_back(define);
            }
        });

        if_contains_key(material_json, "samplerStates", [&](auto& sampler_states) {
            for(auto& sampler_state : sampler_states) {
                ret_val.sampler_states.push_back(decode_sampler_state(sampler_state));
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
}
