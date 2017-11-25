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

        ret_val.states = get_json_value<std::vector<state_enum>>(material_json, "states", [&](const auto& states) {
            auto vec = std::vector<state_enum>{};
            for(auto& state : states) {
                vec.push_back(decode_state(state));
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
                vec.push_back(decode_vertex_field(vertex_field));
            }
            return vec;
        });

        ret_val.front_face = get_json_value<stencil_buffer_state>(material_json, "frontFace", decode_stencil_buffer_state);

        ret_val.back_face = get_json_value<stencil_buffer_state>(material_json, "backFace", decode_stencil_buffer_state);

        ret_val.stencil_ref = get_json_value<uint32_t>(material_json, "stencilRef");

        ret_val.stencil_read_mask = get_json_value<uint32_t>(material_json, "stencilReadMask");

        ret_val.stencil_write_mask = get_json_value<uint32_t>(material_json, "stencilWriteMask");

        ret_val.msaa_support = get_json_value<msaa_support_enum>(material_json, "msaaSupport", decode_msaa_support_enum);

        ret_val.primitive_mode = get_json_value<vk::PrimitiveTopology>(material_json, "primitiveMode", decode_primitive_mode_enum);

        ret_val.source_blend_factor = get_json_value<blend_source_enum>(material_json, "blendSrc", decode_blend_source_enum);

        ret_val.destination_blend_factor = get_json_value<blend_source_enum>(material_json, "blendDst", decode_blend_source_enum);

        ret_val.textures = get_json_value<std::vector<texture>>(material_json, "textures", [&](const nlohmann::json& textures) {
            auto vec = std::vector<texture>{};

            for(const auto& texture_field : textures) {
                vec.push_back(decode_texture(texture_field));
            }

            return vec;
        });

        ret_val.alpha_src = get_json_value<blend_source_enum>(material_json, "alphaSrc", decode_blend_source_enum);
        ret_val.alpha_dst = get_json_value<blend_source_enum>(material_json, "alphaDst", decode_blend_source_enum);
        ret_val.depth_func = get_json_value<vk::CompareOp>(material_json, "depthFunc", decode_comparison_func_enum);

        ret_val.filters = get_json_value<std::string>(material_json, "filters");
        ret_val.fallback = get_json_value<std::string>(material_json, "fallback");
        ret_val.pass_index = get_json_value<uint32_t>(material_json, "passIndex");
        ret_val.has_transparency = get_json_value<bool>(material_json, "hasTransparency");
        ret_val.has_cutout = get_json_value<bool>(material_json, "hasCutout");

        ret_val.outputs = get_json_value<std::vector<uint8_t>>(material_json, "outputs");

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

        } else if(state_to_decode == "DisableDepthTest") {
            return state_enum::disable_depth_test;
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
            return vk::StencilOp::eDecrementAndClamp

        } else if(op == "DecrementAndSwap") {
            return vk::StencilOp::eDecrementAndWrap

        } else if(op == "Invert") {
            return vk::StencilOp::eInvert;

        }

        LOG(FATAL) << "Invalid stencil or depth operation '" << op << "'";
    }

    stencil_buffer_state decode_stencil_buffer_state(const nlohmann::json &json) {
        auto ret_val = stencil_buffer_state{};

        ret_val.stencil_func = get_json_value<vk::CompareOp>(json, "stencilFunc", decode_comparison_func_enum);

        ret_val.stencil_fail_op = get_json_value<vk::StencilOp>(json, "stencilFailOp", decode_stencil_op_enum);

        ret_val.stencil_depth_fail_op = get_json_value<vk::StencilOp>(json, "stencilDepthFailOp", decode_stencil_op_enum);

        ret_val.stencil_pass_op = get_json_value<vk::StencilOp>(json, "stencilPassOp", decode_stencil_op_enum);

        return ret_val;
    }

    msaa_support_enum decode_msaa_support_enum(const std::string& msaa_support_str) {
        if(msaa_support_str == "MSAA") {
            return msaa_support_enum::msaa;

        } else if(msaa_support_str == "Both") {
            return msaa_support_enum::both;
        }

        LOG(FATAL) << "Invalid value for msaaSupport: '" << msaa_support_str << "'";
    }

    vk::PrimitiveTopology decode_primitive_mode_enum(const std::string& primitive_mode_str) {
        if(primitive_mode_str == "Line") {
            return vk::PrimitiveTopology::eLineList;

        } else if(primitive_mode_str == "Triangle") {
            return vk::PrimitiveTopology::eTriangleList;

        }

        LOG(FATAL) << "Invalid primitive mode: '" << primitive_mode_str << "'";
    }

    blend_source_enum decode_blend_source_enum(const std::string& blend_source_str) {
        if(blend_source_str == "SourceColor") {
            return blend_source_enum::source_color;

        } else if(blend_source_str == "Zero") {
            return blend_source_enum::zero;

        } else if(blend_source_str == "One") {
            return blend_source_enum::one;

        } else if(blend_source_str == "SourceAlpha") {
            return blend_source_enum::source_alpha;

        } else if(blend_source_str == "OneMinusSourceAlpha") {
            return blend_source_enum::one_minus_source_alpha;

        } else if(blend_source_str == "DstColor") {
            return blend_source_enum::dest_color;

        } else if(blend_source_str == "OneMinusDstColor") {
            return blend_source_enum::one_minus_dest_color;

        }

        LOG(FATAL) << "Invalid blend source '" << blend_source_str << "'";
    }

    texture decode_texture(const nlohmann::json& texture_json) {
        auto tex = texture{};

        // If we don't have these, it's an error
        // TODO: A better parser for this stuff. Maybe extend optional to have an exception or error string or something?
        tex.index = get_json_value<uint32_t>(texture_json, "textureIndex").value();

        tex.texture_location = get_json_value<texture_location_enum>(texture_json, "textureLocation", decode_texture_location_enum).value();

        tex.texture_name = get_json_value<std::string>(texture_json, "textureName").value();

        tex.calculate_mipmaps = get_json_value<bool>(texture_json, "calculateMipmaps");

        return tex;
    }

    texture_location_enum decode_texture_location_enum(const std::string& texture_location_str) {
        if(texture_location_str == "InUserPackage") {
            return texture_location_enum::in_user_package;

        } else if(texture_location_str == "Dynamic") {
            return texture_location_enum::dynamic;

        }

        LOG(FATAL) << "Invalid texture location enum '" << texture_location_str << "'";
    }
}
