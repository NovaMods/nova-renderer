/*!
 * \author ddubois
 * \date 23-Aug-18.
 */

#include "../../util/vma_usage.hpp"

#include <Common.h>
#include "shaderpack_data.hpp"
#include "../json_utils.hpp"

namespace nova {
    /*!
     * \brief If a data member isn't in the JSON (which is fully supported and is 100% fine) then we use this to fill in
     * any missing values
     */
    pipeline_data default_pipeline;

    bool texture_format::operator==(const texture_format &other) const {
        return pixel_format == other.pixel_format && dimension_type == other.dimension_type && width == other.width && height == other.height;
    }

    bool texture_format::operator!=(const texture_format &other) const {
        return !(*this == other);
    }

    bool texture_attachment::operator==(const texture_attachment& other) const {
        return other.name == name;
    }

    glm::uvec2 texture_format::get_size_in_pixels(const glm::uvec2& screen_size) const {
        float pixel_width = width;
        float pixel_height = height;

        if(dimension_type == texture_dimension_type_enum::ScreenRelative) {
            pixel_width *= screen_size.x;
            pixel_height *= screen_size.y;
        }

        return {std::round(pixel_width), std::round(pixel_height)};
    }

    pixel_format_enum pixel_format_enum_from_string(const std::string &str) {
        if(str == "RGBA8") {
            return pixel_format_enum::RGBA8;

        } else if(str == "RGBA16F") {
            return pixel_format_enum::RGBA16F;

        } else if(str == "RGBA32F") {
            return pixel_format_enum::RGBA32F;

        } else if(str == "Depth") {
            return pixel_format_enum::Depth;

        } else if(str == "DepthStencil") {
            return pixel_format_enum::DepthStencil;

        } else {
            NOVA_LOG(ERROR) << "Unsupported pixel format " << str;
            throw validation_failure_exception("Unsupported pixel format " + str);
        }
    }

    texture_dimension_type_enum texture_dimension_type_enum_from_string(const std::string &str) {
        if(str == "ScreenRelative") {
            return texture_dimension_type_enum ::ScreenRelative;

        } else if(str == "Absolute") {
            return texture_dimension_type_enum::Absolute;

        } else {
            NOVA_LOG(ERROR) << "Unsupported texture dimension type " << str;
            throw validation_failure_exception("Unsupported texture dimension type " + str);
        }
    }

    texture_filter_enum texture_filter_enum_from_string(const std::string &str) {
        if(str == "TexelAA") {
            return texture_filter_enum::TexelAA;

        } else if(str == "Bilinear") {
            return texture_filter_enum::Bilinear;

        } else if(str == "Point") {
            return texture_filter_enum::Point;

        } else {
            NOVA_LOG(ERROR) << "Unsupported texture filter " << str;
            throw validation_failure_exception("Unsupported texture filter " + str);
        }
    }

    wrap_mode_enum wrap_mode_enum_from_string(const std::string &str) {
        if(str == "Repeat") {
            return wrap_mode_enum::Repeat;

        } else if(str == "Clamp") {
            return wrap_mode_enum::Clamp;

        } else {
            NOVA_LOG(ERROR) << "Unsupported wrap mode " << str;
            throw validation_failure_exception("Unsupported wrap mode " + str);
        }
    }

    stencil_op_enum stencil_op_enum_from_string(const std::string &str) {
        if(str == "Keep") {
            return stencil_op_enum::Keep;

        } else if(str == "Zero") {
            return stencil_op_enum::Zero;

        } else if(str == "Replace") {
            return stencil_op_enum::Replace;

        } else if(str == "Incr") {
            return stencil_op_enum::Incr;

        } else if(str == "IncrWrap") {
            return stencil_op_enum::IncrWrap;

        } else if(str == "Decr") {
            return stencil_op_enum::Decr;

        } else if(str == "DecrWrap") {
            return stencil_op_enum::DecrWrap;

        } else if(str == "Invert") {
            return stencil_op_enum::Invert;

        } else {
            NOVA_LOG(ERROR) << "Unsupported stencil op " << str;
            throw validation_failure_exception("Unsupported stencil op " + str);
        }
    }

    compare_op_enum compare_op_enum_from_string(const std::string &str) {
        if(str == "Never") {
            return compare_op_enum::Never;

        } else if(str == "Less") {
            return compare_op_enum::Less;

        } else if(str == "LessEqual") {
            return compare_op_enum::LessEqual;

        } else if(str == "Greater") {
            return compare_op_enum::Greater;

        } else if(str == "GreaterEqual") {
            return compare_op_enum::GreaterEqual;

        } else if(str == "Equal") {
            return compare_op_enum::Equal;

        } else if(str == "NotEqual") {
            return compare_op_enum::NotEqual;

        } else if(str == "Always") {
            return compare_op_enum::Always;

        } else {
            NOVA_LOG(ERROR) << "Unsupported compare op " << str;
            throw validation_failure_exception("Unsupported compare op " + str);
        }
    }

    msaa_support_enum msaa_support_enum_from_string(const std::string &str) {
        if(str == "MSAA") {
            return msaa_support_enum::MSAA;

        } else if(str == "Both") {
            return msaa_support_enum::Both;

        } else if(str == "None") {
            return msaa_support_enum::None;

        } else {
            NOVA_LOG(ERROR) << "Unsupported antialiasing mode " << str;
            throw validation_failure_exception("Unsupported antialiasing mode " + str);
        }
    }

    primitive_topology_enum primitive_topology_enum_from_string(const std::string &str) {
        if(str == "Triangles") {
            return primitive_topology_enum::Triangles;

        } else if(str == "Lines") {
            return primitive_topology_enum::Lines;

        } else {
            NOVA_LOG(ERROR) << "Unsupported primitive mode " << str;
            throw validation_failure_exception("Unsupported primitive mode " + str);
        }
    }

    blend_factor_enum blend_factor_enum_from_string(const std::string &str) {
        if(str == "One") {
            return blend_factor_enum::One;

        } else if(str == "Zero") {
            return blend_factor_enum::Zero;

        } else if(str == "SrcColor") {
            return blend_factor_enum::SrcColor;

        } else if(str == "DstColor") {
            return blend_factor_enum::DstColor;

        } else if(str == "OneMinusSrcColor") {
            return blend_factor_enum::OneMinusSrcColor;

        } else if(str == "OneMinusDstColor") {
            return blend_factor_enum::OneMinusDstColor;

        } else if(str == "SrcAlpha") {
            return blend_factor_enum::SrcAlpha;

        } else if(str == "DstAlpha") {
            return blend_factor_enum::DstAlpha;

        } else if(str == "OneMinusSrcAlpha") {
            return blend_factor_enum::OneMinusSrcAlpha;

        } else if(str == "OneMinusDstAlpha") {
            return blend_factor_enum::OneMinusDstAlpha;

        } else {
            NOVA_LOG(ERROR) << "Unsupported blend factor " << str;
            throw validation_failure_exception("Unsupported blend factor " + str);
        }
    }

    render_queue_enum render_queue_enum_from_string(const std::string &str) {
        if(str == "Transparent") {
            return render_queue_enum::Transparent;

        } else if(str == "Opaque") {
            return render_queue_enum::Opaque;

        } else if(str == "Cutout") {
            return render_queue_enum::Cutout;

        } else {
            NOVA_LOG(ERROR) << "Unsupported render queue " << str;
            throw validation_failure_exception("Unsupported render queue " + str);
        }
    }

    state_enum state_enum_from_string(const std::string &str) {
        if(str == "Blending") {
            return state_enum::Blending;

        } else if(str == "InvertCulling") {
            return state_enum::InvertCulling;

        } else if(str == "DisableCulling") {
            return state_enum::DisableCulling;

        } else if(str == "DisableDepthWrite") {
            return state_enum::DisableDepthWrite;

        } else if(str == "DisableDepthTest") {
            return state_enum::DisableDepthTest;

        } else if(str == "EnableStencilTest") {
            return state_enum::EnableStencilTest;

        } else if(str == "StencilWrite") {
            return state_enum::StencilWrite;

        } else if(str == "DisableColorWrite") {
            return state_enum::DisableColorWrite;

        } else if(str == "EnableAlphaToCoverage") {
            return state_enum::EnableAlphaToCoverage;

        } else if(str == "DisableAlphaWrite") {
            return state_enum::DisableAlphaWrite;

        } else {
            NOVA_LOG(ERROR) << "Unsupported state enum " << str;
            throw validation_failure_exception("Unsupported state enum " + str);
        }
    }

    vertex_field_enum vertex_field_enum_from_string(const std::string &str) {
        if(str == "Position") {
            return vertex_field_enum::Position;

        } else if(str == "Color") {
            return vertex_field_enum::Color;

        } else if(str == "UV0") {
            return vertex_field_enum::UV0;

        } else if(str == "UV1") {
            return vertex_field_enum::UV1;

        } else if(str == "Normal") {
            return vertex_field_enum::Normal;

        } else if(str == "Tangent") {
            return vertex_field_enum::Tangent;

        } else if(str == "MidTexCoord") {
            return vertex_field_enum::MidTexCoord;

        } else if(str == "VirtualTextureId") {
            return vertex_field_enum::VirtualTextureId;

        } else if(str == "McEntityId") {
            return vertex_field_enum::McEntityId;

        } else {
            NOVA_LOG(ERROR) << "Unsupported vertex field " << str;
            throw validation_failure_exception("Unsupported vertex field " + str);
        }
    }

    std::string to_string(const pixel_format_enum val) {
        switch(val) {
            case pixel_format_enum::RGBA8: return "RGBA8";

            case pixel_format_enum::RGBA16F: return "RGBA16F";

            case pixel_format_enum::RGBA32F: return "RGBA32F";

            case pixel_format_enum::Depth: return "Depth";

            case pixel_format_enum::DepthStencil: return "DepthStencil";
        }

        return "Unknown value";
    }

    std::string to_string(const texture_dimension_type_enum val) {
        switch(val) {
            case texture_dimension_type_enum::ScreenRelative: return "ScreenRelative";

            case texture_dimension_type_enum::Absolute: return "Absolute";
        }

        return "Unknown value";
    }

    std::string to_string(const texture_filter_enum val) {
        switch(val) {
            case texture_filter_enum::TexelAA: return "TexelAA";

            case texture_filter_enum::Bilinear: return "Bilinear";

            case texture_filter_enum::Point: return "Point";
        }

        return "Unknown value";
    }

    std::string to_string(const wrap_mode_enum val) {
        switch(val) {
            case wrap_mode_enum::Repeat: return "Repeat";

            case wrap_mode_enum::Clamp: return "Clamp";
        }

        return "Unknown value";
    }

    std::string to_string(const stencil_op_enum val) {
        switch(val) {
            case stencil_op_enum::Keep: return "Keep";

            case stencil_op_enum::Zero: return "Zero";

            case stencil_op_enum::Replace: return "Replace";

            case stencil_op_enum::Incr: return "Incr";

            case stencil_op_enum::IncrWrap: return "IncrWrap";

            case stencil_op_enum::Decr: return "Decr";

            case stencil_op_enum::DecrWrap: return "DecrWrap";

            case stencil_op_enum::Invert: return "Invert";
        }

        return "Unknown value";
    }

    std::string to_string(const compare_op_enum val) {
        switch(val) {
            case compare_op_enum::Never: return "Never";

            case compare_op_enum::Less: return "Less";

            case compare_op_enum::LessEqual: return "LessEqual";

            case compare_op_enum::Greater: return "Greater";

            case compare_op_enum::GreaterEqual: return "GreaterEqual";

            case compare_op_enum::Equal: return "Equal";

            case compare_op_enum::NotEqual: return "NotEqual";

            case compare_op_enum::Always: return "Always";
        }

        return "Unknown value";
    }

    std::string to_string(const msaa_support_enum val) {
        switch(val) {
            case msaa_support_enum::MSAA: return "MSAA";

            case msaa_support_enum::Both: return "Both";

            case msaa_support_enum::None: return "None";
        }

        return "Unknown value";
    }

    std::string to_string(const primitive_topology_enum val) {
        switch(val) {
            case primitive_topology_enum::Triangles: return "Triangles";

            case primitive_topology_enum::Lines: return "Lines";
        }

        return "Unknown value";
    }

    std::string to_string(const blend_factor_enum val) {
        switch(val) {
            case blend_factor_enum::One: return "One";

            case blend_factor_enum::Zero: return "Zero";

            case blend_factor_enum::SrcColor: return "SrcColor";

            case blend_factor_enum::DstColor: return "DstColor";

            case blend_factor_enum::OneMinusSrcColor: return "OneMinusSrcColor";

            case blend_factor_enum::OneMinusDstColor: return "OneMinusDstColor";

            case blend_factor_enum::SrcAlpha: return "SrcAlpha";

            case blend_factor_enum::DstAlpha: return "DstAlpha";

            case blend_factor_enum::OneMinusSrcAlpha: return "OneMinusSrcAlpha";

            case blend_factor_enum::OneMinusDstAlpha: return "OneMinusDstAlpha";
        }

        return "Unknown value";
    }

    std::string to_string(const render_queue_enum val) {
        switch(val) {
            case render_queue_enum::Transparent: return "Transparent";

            case render_queue_enum::Opaque: return "Opaque";

            case render_queue_enum::Cutout: return "Cutout";
        }

        return "Unknown value";
    }

    std::string to_string(const state_enum val) {
        switch(val) {
            case state_enum::Blending: return "Blending";

            case state_enum::InvertCulling: return "InvertCulling";

            case state_enum::DisableCulling: return "DisableCulling";

            case state_enum::DisableDepthWrite: return "DisableDepthWrite";

            case state_enum::DisableDepthTest: return "DisableDepthTest";

            case state_enum::EnableStencilTest: return "EnableStencilTest";

            case state_enum::StencilWrite: return "StencilWrite";

            case state_enum::DisableColorWrite: return "DisableColorWrite";

            case state_enum::EnableAlphaToCoverage: return "EnableAlphaToCoverage";

            case state_enum::DisableAlphaWrite: return "DisableAlphaWrite";
        }

        return "Unknown value";
    }

    std::string to_string(const vertex_field_enum val) {
        switch(val) {
            case vertex_field_enum::Position: return "Position";

            case vertex_field_enum::Color: return "Color";

            case vertex_field_enum::UV0: return "UV0";

            case vertex_field_enum::UV1: return "UV1";

            case vertex_field_enum::Normal: return "Normal";

            case vertex_field_enum::Tangent: return "Tangent";

            case vertex_field_enum::MidTexCoord: return "MidTexCoord";

            case vertex_field_enum::VirtualTextureId: return "VirtualTextureId";

            case vertex_field_enum::McEntityId: return "McEntityId";
        }

        return "Unknown value";
    }
}  // namespace nova
