#include "nova_renderer/shaderpack_data.hpp"

#include <rx/core/log.h>

#include "nova_renderer/rhi/rhi_enums.hpp"

#include "../json_utils.hpp"

#define FILL_REQUIRED_FIELD(field, expr)                                                                                                   \
    [&] {                                                                                                                                  \
        const auto val = expr;                                                                                                             \
        if(val) {                                                                                                                          \
            (field) = *val;                                                                                                                \
        }                                                                                                                                  \
    }();

namespace nova::renderer::shaderpack {
    RX_LOG("ShaderpackData", logger);

    bool TextureFormat::operator==(const TextureFormat& other) const {
        return pixel_format == other.pixel_format && dimension_type == other.dimension_type && width == other.width &&
               height == other.height;
    }

    bool TextureFormat::operator!=(const TextureFormat& other) const { return !(*this == other); }

    TextureFormat TextureFormat::from_json(const rx::json& json) {
        TextureFormat format = {};

        format.pixel_format = get_json_value(json, "pixelFormat", PixelFormatEnum::RGBA8, pixel_format_enum_from_json);
        format.dimension_type = get_json_value<TextureDimensionTypeEnum>(json,
                                                                         "dimensionType",
                                                                         TextureDimensionTypeEnum::ScreenRelative,
                                                                         texture_dimension_type_enum_from_json);
        format.width = get_json_value<float>(json, "width", 0);
        format.height = get_json_value<float>(json, "height", 0);

        return format;
    }

    TextureCreateInfo TextureCreateInfo::from_json(const rx::json& json) {
        TextureCreateInfo info = {};

        FILL_REQUIRED_FIELD(info.name, get_json_opt<rx::string>(json, "name"));
        FILL_REQUIRED_FIELD(info.format, get_json_opt<TextureFormat>(json, "format"));

        return info;
    }

    ShaderpackResourcesData ShaderpackResourcesData::from_json(const rx::json& json) {
        ShaderpackResourcesData data;
        data.render_targets = get_json_array<TextureCreateInfo>(json, "textures");
        data.samplers = get_json_array<SamplerCreateInfo>(json, "samplers");

        // TODO: buffers
        // TODO: arbitrary images

        return data;
    }

    bool TextureAttachmentInfo::operator==(const TextureAttachmentInfo& other) const { return other.name == name; }

    TextureAttachmentInfo TextureAttachmentInfo::from_json(const rx::json& json) {
        TextureAttachmentInfo info = {};

        FILL_REQUIRED_FIELD(info.name, get_json_opt<rx::string>(json, "name"));
        info.clear = get_json_value(json, "clear", false);

        return info;
    }

    RenderPassCreateInfo RenderPassCreateInfo::from_json(const rx::json& json) {
        RenderPassCreateInfo info = {};

        info.texture_inputs = get_json_array<rx::string>(json, "textureInputs");
        info.texture_outputs = get_json_array<TextureAttachmentInfo>(json, "textureOutputs");
        info.depth_texture = get_json_opt<TextureAttachmentInfo>(json, "depthTexture");

        info.input_buffers = get_json_array<rx::string>(json, "inputBuffers");
        info.output_buffers = get_json_array<rx::string>(json, "outputBuffers");

        info.name = get_json_value<rx::string>(json, "name", "<NAME_MISSING>");

        return info;
    }

    RendergraphData RendergraphData::from_json(const rx::json& json) {
        RendergraphData data;

        data.passes = get_json_array<RenderPassCreateInfo>(json, "passes");
        data.builtin_passes = get_json_array<rx::string>(json, "builtinPasses");

        return data;
    }

    SamplerCreateInfo SamplerCreateInfo::from_json(const rx::json& json) {
        SamplerCreateInfo info = {};

        info.filter = get_json_value(json, "filter", TextureFilterEnum::Point, texture_filter_enum_from_json);
        info.wrap_mode = get_json_value(json, "wrapMode", WrapModeEnum::Clamp, wrap_mode_enum_from_json);

        return info;
    }

    StencilOpState StencilOpState::from_json(const rx::json& json) {
        StencilOpState state = {};

        FILL_REQUIRED_FIELD(state.fail_op, get_json_opt<StencilOpEnum>(json, "failOp", stencil_op_enum_from_json));
        FILL_REQUIRED_FIELD(state.pass_op, get_json_opt<StencilOpEnum>(json, "passOp", stencil_op_enum_from_json));
        FILL_REQUIRED_FIELD(state.depth_fail_op, get_json_opt<StencilOpEnum>(json, "depthFailOp", stencil_op_enum_from_json));
        FILL_REQUIRED_FIELD(state.compare_op, get_json_opt<CompareOpEnum>(json, "compareOp", compare_op_enum_from_json));
        FILL_REQUIRED_FIELD(state.compare_mask, get_json_opt<uint32_t>(json, "compareMask"));
        FILL_REQUIRED_FIELD(state.write_mask, get_json_opt<uint32_t>(json, "writeMask"));

        return state;
    }

    PipelineCreateInfo PipelineCreateInfo::from_json(const rx::json& json) {
        PipelineCreateInfo pipeline = {};

        FILL_REQUIRED_FIELD(pipeline.name, get_json_opt<rx::string>(json, "name"));
        FILL_REQUIRED_FIELD(pipeline.pass, get_json_opt<rx::string>(json, "pass"));
        pipeline.parent_name = get_json_value(json, "parent", "");

        pipeline.defines = get_json_array<rx::string>(json, "defined");

        pipeline.states = get_json_array<StateEnum>(json, "states", state_enum_from_json);
        pipeline.front_face = get_json_opt<StencilOpState>(json, "frontFace");
        pipeline.back_face = get_json_opt<StencilOpState>(json, "backFace");
        pipeline.fallback = get_json_value<rx::string>(json, "fallback", {});
        pipeline.depth_bias = get_json_value<float>(json, "depthBias", 0);
        pipeline.slope_scaled_depth_bias = get_json_value<float>(json, "slopeScaledDepthBias", 0);
        pipeline.stencil_ref = get_json_value<uint32_t>(json, "stencilRef", 0);
        pipeline.stencil_read_mask = get_json_value<uint32_t>(json, "stencilReadMask", 0);
        pipeline.stencil_write_mask = get_json_value<uint32_t>(json, "stencilWriteMask", 0);
        pipeline.msaa_support = get_json_value<MsaaSupportEnum>(json, "msaaSupport", MsaaSupportEnum::None, msaa_support_enum_from_json);
        pipeline.primitive_mode = get_json_value<PrimitiveTopologyEnum>(json,
                                                                        "primitiveMode",
                                                                        PrimitiveTopologyEnum::Triangles,
                                                                        primitive_topology_enum_from_json);
        pipeline.source_color_blend_factor = get_json_value<BlendFactorEnum>(json,
                                                                             "sourceBlendFactor",
                                                                             BlendFactorEnum::One,
                                                                             blend_factor_enum_from_json);
        pipeline.destination_color_blend_factor = get_json_value<BlendFactorEnum>(json,
                                                                                  "destBlendFactor",
                                                                                  BlendFactorEnum::Zero,
                                                                                  blend_factor_enum_from_json);
        pipeline.source_alpha_blend_factor = get_json_value<BlendFactorEnum>(json,
                                                                             "alphaSrc",
                                                                             BlendFactorEnum::One,
                                                                             blend_factor_enum_from_json);
        pipeline.destination_alpha_blend_factor = get_json_value<BlendFactorEnum>(json,
                                                                                  "alphaDest",
                                                                                  BlendFactorEnum::Zero,
                                                                                  blend_factor_enum_from_json);
        pipeline.depth_func = get_json_value<CompareOpEnum>(json, "depthFunc", CompareOpEnum::Less, compare_op_enum_from_json);
        pipeline.render_queue = get_json_value<RenderQueueEnum>(json, "renderQueue", RenderQueueEnum::Opaque, render_queue_enum_from_json);

        pipeline.vertex_shader.filename = get_json_value<rx::string>(json, "vertexShader", "<NAME_MISSING>");

        const auto geometry_shader_name = get_json_opt<rx::string>(json, "geometryShader");
        if(geometry_shader_name) {
            pipeline.geometry_shader = ShaderSource{};
            pipeline.geometry_shader->filename = *geometry_shader_name;
        }

        const auto tess_control_shader_name = get_json_opt<rx::string>(json, "tessellationControlShader");
        if(tess_control_shader_name) {
            pipeline.tessellation_control_shader = ShaderSource{};
            pipeline.tessellation_control_shader->filename = *tess_control_shader_name;
        }

        const auto tess_eval_shader_name = get_json_opt<rx::string>(json, "tessellationEvalShader");
        if(tess_eval_shader_name) {
            pipeline.tessellation_evaluation_shader = ShaderSource{};
            pipeline.tessellation_evaluation_shader->filename = *tess_eval_shader_name;
        }

        const auto fragment_shader_name = get_json_opt<rx::string>(json, "fragmentShader");
        if(fragment_shader_name) {
            pipeline.fragment_shader = ShaderSource{};
            pipeline.fragment_shader->filename = *fragment_shader_name;
        }

        return pipeline;
    }

    glm::uvec2 TextureFormat::get_size_in_pixels(const glm::uvec2& screen_size) const {
        float pixel_width = width;
        float pixel_height = height;

        if(dimension_type == TextureDimensionTypeEnum::ScreenRelative) {
            pixel_width *= static_cast<float>(screen_size.x);
            pixel_height *= static_cast<float>(screen_size.y);
        }

        return {std::round(pixel_width), std::round(pixel_height)};
    }

    rx::optional<rx::map<rx::string, rx::string>> map_from_json_object(const rx::json& json) {
        rx::map<rx::string, rx::string> map;

        json.each([&](const rx::json& elem) {
            rx::string shader_variable;
            FILL_REQUIRED_FIELD(shader_variable, get_json_opt<rx::string>(elem, "variable"));

            rx::string resource_name;
            FILL_REQUIRED_FIELD(resource_name, get_json_opt<rx::string>(elem, "resource"));

            map.insert(shader_variable, resource_name);
        });

        return map;
    }

    MaterialPass MaterialPass::from_json(const rx::json& json) {
        MaterialPass pass = {};

        FILL_REQUIRED_FIELD(pass.name, get_json_opt<rx::string>(json, "name"));
        FILL_REQUIRED_FIELD(pass.pipeline, get_json_opt<rx::string>(json, "pipeline"));

        const auto val = get_json_opt<rx::map<rx::string, rx::string>>(json, "bindings", map_from_json_object);
        if(val) {
            pass.bindings = *val;
        }

        // FILL_REQUIRED_FIELD(pass.bindings, get_json_opt<rx::map<rx::string, rx::string>>(json, "bindings", map_from_json_object));

        return pass;
    }

    MaterialData MaterialData::from_json(const rx::json& json) {
        MaterialData data = {};

        FILL_REQUIRED_FIELD(data.name, get_json_opt<rx::string>(json, "name"));
        data.passes = get_json_array<MaterialPass>(json, "passes");
        FILL_REQUIRED_FIELD(data.geometry_filter, get_json_opt<rx::string>(json, "filter"));

        return data;
    }

    PixelFormatEnum pixel_format_enum_from_string(const rx::string& str) {
        if(str == "RGBA8") {
            return PixelFormatEnum::RGBA8;
        }
        if(str == "RGBA16F") {
            return PixelFormatEnum::RGBA16F;
        }
        if(str == "RGBA32F") {
            return PixelFormatEnum::RGBA32F;
        }
        if(str == "Depth") {
            return PixelFormatEnum::Depth;
        }
        if(str == "DepthStencil") {
            return PixelFormatEnum::DepthStencil;
        }

        logger(rx::log::level::k_error, "Unsupported pixel format %s", str);
        return {};
    }

    TextureDimensionTypeEnum texture_dimension_type_enum_from_string(const rx::string& str) {
        if(str == "ScreenRelative") {
            return TextureDimensionTypeEnum ::ScreenRelative;
        }
        if(str == "Absolute") {
            return TextureDimensionTypeEnum::Absolute;
        }

        logger(rx::log::level::k_error, "Unsupported texture dimension type %s", str);
        return {};
    }

    TextureFilterEnum texture_filter_enum_from_string(const rx::string& str) {
        if(str == "TexelAA") {
            return TextureFilterEnum::TexelAA;
        }
        if(str == "Bilinear") {
            return TextureFilterEnum::Bilinear;
        }
        if(str == "Point") {
            return TextureFilterEnum::Point;
        }

        logger(rx::log::level::k_error, "Unsupported texture filter %s", str);
        return {};
    }

    WrapModeEnum wrap_mode_enum_from_string(const rx::string& str) {
        if(str == "Repeat") {
            return WrapModeEnum::Repeat;
        }
        if(str == "Clamp") {
            return WrapModeEnum::Clamp;
        }

        logger(rx::log::level::k_error, "Unsupported wrap mode %s", str);
        return {};
    }

    StencilOpEnum stencil_op_enum_from_string(const rx::string& str) {
        if(str == "Keep") {
            return StencilOpEnum::Keep;
        }
        if(str == "Zero") {
            return StencilOpEnum::Zero;
        }
        if(str == "Replace") {
            return StencilOpEnum::Replace;
        }
        if(str == "Incr") {
            return StencilOpEnum::Increment;
        }
        if(str == "IncrWrap") {
            return StencilOpEnum::IncrementAndWrap;
        }
        if(str == "Decr") {
            return StencilOpEnum::Decrement;
        }
        if(str == "DecrWrap") {
            return StencilOpEnum::DecrementAndWrap;
        }
        if(str == "Invert") {
            return StencilOpEnum::Invert;
        }

        logger(rx::log::level::k_error, "Unsupported stencil op %s", str);
        return {};
    }

    CompareOpEnum compare_op_enum_from_string(const rx::string& str) {
        if(str == "Never") {
            return CompareOpEnum::Never;
        }
        if(str == "Less") {
            return CompareOpEnum::Less;
        }
        if(str == "LessEqual") {
            return CompareOpEnum::LessEqual;
        }
        if(str == "Greater") {
            return CompareOpEnum::Greater;
        }
        if(str == "GreaterEqual") {
            return CompareOpEnum::GreaterEqual;
        }
        if(str == "Equal") {
            return CompareOpEnum::Equal;
        }
        if(str == "NotEqual") {
            return CompareOpEnum::NotEqual;
        }
        if(str == "Always") {
            return CompareOpEnum::Always;
        }

        logger(rx::log::level::k_error, "Unsupported compare op ", str);
        return {};
    }

    MsaaSupportEnum msaa_support_enum_from_string(const rx::string& str) {
        if(str == "MSAA") {
            return MsaaSupportEnum::MSAA;
        }
        if(str == "Both") {
            return MsaaSupportEnum::Both;
        }
        if(str == "None") {
            return MsaaSupportEnum::None;
        }

        logger(rx::log::level::k_error, "Unsupported antialiasing mode %s", str);
        return {};
    }

    PrimitiveTopologyEnum primitive_topology_enum_from_string(const rx::string& str) {
        if(str == "Triangles") {
            return PrimitiveTopologyEnum::Triangles;
        }
        if(str == "Lines") {
            return PrimitiveTopologyEnum::Lines;
        }

        logger(rx::log::level::k_error, "Unsupported primitive mode %s", str);
        return {};
    }

    BlendFactorEnum blend_factor_enum_from_string(const rx::string& str) {
        if(str == "One") {
            return BlendFactorEnum::One;
        }
        if(str == "Zero") {
            return BlendFactorEnum::Zero;
        }
        if(str == "SrcColor") {
            return BlendFactorEnum::SrcColor;
        }
        if(str == "DstColor") {
            return BlendFactorEnum::DstColor;
        }
        if(str == "OneMinusSrcColor") {
            return BlendFactorEnum::OneMinusSrcColor;
        }
        if(str == "OneMinusDstColor") {
            return BlendFactorEnum::OneMinusDstColor;
        }
        if(str == "SrcAlpha") {
            return BlendFactorEnum::SrcAlpha;
        }
        if(str == "DstAlpha") {
            return BlendFactorEnum::DstAlpha;
        }
        if(str == "OneMinusSrcAlpha") {
            return BlendFactorEnum::OneMinusSrcAlpha;
        }
        if(str == "OneMinusDstAlpha") {
            return BlendFactorEnum::OneMinusDstAlpha;
        }

        logger(rx::log::level::k_error, "Unsupported blend factor %s", str);
        return {};
    }

    RenderQueueEnum render_queue_enum_from_string(const rx::string& str) {
        if(str == "Transparent") {
            return RenderQueueEnum::Transparent;
        }
        if(str == "Opaque") {
            return RenderQueueEnum::Opaque;
        }
        if(str == "Cutout") {
            return RenderQueueEnum::Cutout;
        }

        logger(rx::log::level::k_error, "Unsupported render queue %s", str);
        return {};
    }

    StateEnum state_enum_from_string(const rx::string& str) {
        if(str == "Blending") {
            return StateEnum::Blending;
        }
        if(str == "InvertCulling") {
            return StateEnum::InvertCulling;
        }
        if(str == "DisableCulling") {
            return StateEnum::DisableCulling;
        }
        if(str == "DisableDepthWrite") {
            return StateEnum::DisableDepthWrite;
        }
        if(str == "DisableDepthTest") {
            return StateEnum::DisableDepthTest;
        }
        if(str == "EnableStencilTest") {
            return StateEnum::EnableStencilTest;
        }
        if(str == "StencilWrite") {
            return StateEnum::StencilWrite;
        }
        if(str == "DisableColorWrite") {
            return StateEnum::DisableColorWrite;
        }
        if(str == "EnableAlphaToCoverage") {
            return StateEnum::EnableAlphaToCoverage;
        }
        if(str == "DisableAlphaWrite") {
            return StateEnum::DisableAlphaWrite;
        }

        logger(rx::log::level::k_error, "Unsupported state enum %s", str);
        return {};
    }

    PixelFormatEnum pixel_format_enum_from_json(const rx::json& j) { return pixel_format_enum_from_string(j.as_string()); }

    TextureDimensionTypeEnum texture_dimension_type_enum_from_json(const rx::json& j) {
        return texture_dimension_type_enum_from_string(j.as_string());
    }

    TextureFilterEnum texture_filter_enum_from_json(const rx::json& j) { return texture_filter_enum_from_string(j.as_string()); }

    WrapModeEnum wrap_mode_enum_from_json(const rx::json& j) { return wrap_mode_enum_from_string(j.as_string()); }

    StencilOpEnum stencil_op_enum_from_json(const rx::json& j) { return stencil_op_enum_from_string(j.as_string()); }

    CompareOpEnum compare_op_enum_from_json(const rx::json& j) { return compare_op_enum_from_string(j.as_string()); }

    MsaaSupportEnum msaa_support_enum_from_json(const rx::json& j) { return msaa_support_enum_from_string(j.as_string()); }

    PrimitiveTopologyEnum primitive_topology_enum_from_json(const rx::json& j) {
        return primitive_topology_enum_from_string(j.as_string());
    }

    BlendFactorEnum blend_factor_enum_from_json(const rx::json& j) { return blend_factor_enum_from_string(j.as_string()); }

    RenderQueueEnum render_queue_enum_from_json(const rx::json& j) { return render_queue_enum_from_string(j.as_string()); }

    StateEnum state_enum_from_json(const rx::json& j) { return state_enum_from_string(j.as_string()); }

    rx::string to_string(const PixelFormatEnum val) {
        switch(val) {
            case PixelFormatEnum::RGBA8:
                return "RGBA8";

            case PixelFormatEnum::RGBA16F:
                return "RGBA16F";

            case PixelFormatEnum::RGBA32F:
                return "RGBA32F";

            case PixelFormatEnum::Depth:
                return "Depth";

            case PixelFormatEnum::DepthStencil:
                return "DepthStencil";
        }

        return "Unknown value";
    }

    rx::string to_string(const TextureDimensionTypeEnum val) {
        switch(val) {
            case TextureDimensionTypeEnum::ScreenRelative:
                return "ScreenRelative";

            case TextureDimensionTypeEnum::Absolute:
                return "Absolute";
        }

        return "Unknown value";
    }

    rx::string to_string(const TextureFilterEnum val) {
        switch(val) {
            case TextureFilterEnum::TexelAA:
                return "TexelAA";

            case TextureFilterEnum::Bilinear:
                return "Bilinear";

            case TextureFilterEnum::Point:
                return "Point";
        }

        return "Unknown value";
    }

    rx::string to_string(const WrapModeEnum val) {
        switch(val) {
            case WrapModeEnum::Repeat:
                return "Repeat";

            case WrapModeEnum::Clamp:
                return "Clamp";
        }

        return "Unknown value";
    }

    rx::string to_string(const StencilOpEnum val) {
        switch(val) {
            case StencilOpEnum::Keep:
                return "Keep";

            case StencilOpEnum::Zero:
                return "Zero";

            case StencilOpEnum::Replace:
                return "Replace";

            case StencilOpEnum::Increment:
                return "Incr";

            case StencilOpEnum::IncrementAndWrap:
                return "IncrWrap";

            case StencilOpEnum::Decrement:
                return "Decr";

            case StencilOpEnum::DecrementAndWrap:
                return "DecrWrap";

            case StencilOpEnum::Invert:
                return "Invert";
        }

        return "Unknown value";
    }

    rx::string to_string(const CompareOpEnum val) {
        switch(val) {
            case CompareOpEnum::Never:
                return "Never";

            case CompareOpEnum::Less:
                return "Less";

            case CompareOpEnum::LessEqual:
                return "LessEqual";

            case CompareOpEnum::Greater:
                return "Greater";

            case CompareOpEnum::GreaterEqual:
                return "GreaterEqual";

            case CompareOpEnum::Equal:
                return "Equal";

            case CompareOpEnum::NotEqual:
                return "NotEqual";

            case CompareOpEnum::Always:
                return "Always";
        }

        return "Unknown value";
    }

    rx::string to_string(const MsaaSupportEnum val) {
        switch(val) {
            case MsaaSupportEnum::MSAA:
                return "MSAA";

            case MsaaSupportEnum::Both:
                return "Both";

            case MsaaSupportEnum::None:
                return "None";
        }

        return "Unknown value";
    }

    rx::string to_string(const PrimitiveTopologyEnum val) {
        switch(val) {
            case PrimitiveTopologyEnum::Triangles:
                return "Triangles";

            case PrimitiveTopologyEnum::Lines:
                return "Lines";
        }

        return "Unknown value";
    }

    rx::string to_string(const BlendFactorEnum val) {
        switch(val) {
            case BlendFactorEnum::One:
                return "One";

            case BlendFactorEnum::Zero:
                return "Zero";

            case BlendFactorEnum::SrcColor:
                return "SrcColor";

            case BlendFactorEnum::DstColor:
                return "DstColor";

            case BlendFactorEnum::OneMinusSrcColor:
                return "OneMinusSrcColor";

            case BlendFactorEnum::OneMinusDstColor:
                return "OneMinusDstColor";

            case BlendFactorEnum::SrcAlpha:
                return "SrcAlpha";

            case BlendFactorEnum::DstAlpha:
                return "DstAlpha";

            case BlendFactorEnum::OneMinusSrcAlpha:
                return "OneMinusSrcAlpha";

            case BlendFactorEnum::OneMinusDstAlpha:
                return "OneMinusDstAlpha";
        }

        return "Unknown value";
    }

    rx::string to_string(const RenderQueueEnum val) {
        switch(val) {
            case RenderQueueEnum::Transparent:
                return "Transparent";

            case RenderQueueEnum::Opaque:
                return "Opaque";

            case RenderQueueEnum::Cutout:
                return "Cutout";
        }

        return "Unknown value";
    }

    rx::string to_string(const StateEnum val) {
        switch(val) {
            case StateEnum::Blending:
                return "Blending";

            case StateEnum::InvertCulling:
                return "InvertCulling";

            case StateEnum::DisableCulling:
                return "DisableCulling";

            case StateEnum::DisableDepthWrite:
                return "DisableDepthWrite";

            case StateEnum::DisableDepthTest:
                return "DisableDepthTest";

            case StateEnum::EnableStencilTest:
                return "EnableStencilTest";

            case StateEnum::StencilWrite:
                return "StencilWrite";

            case StateEnum::DisableColorWrite:
                return "DisableColorWrite";

            case StateEnum::EnableAlphaToCoverage:
                return "EnableAlphaToCoverage";

            case StateEnum::DisableAlphaWrite:
                return "DisableAlphaWrite";
        }

        return "Unknown value";
    }

    uint32_t pixel_format_to_pixel_width(const PixelFormatEnum format) {
        switch(format) {
            case PixelFormatEnum::RGBA8:
                return 4 * 8;

            case PixelFormatEnum::RGBA16F:
                return 4 * 16;

            case PixelFormatEnum::RGBA32F:
                return 4 * 32;

            case PixelFormatEnum::Depth:
                return 32;

            case PixelFormatEnum::DepthStencil:
                return 32;

            default:
                return 32;
        }
    }

    PixelFormatEnum to_pixel_format_enum(const rhi::PixelFormat format) {
        switch(format) {
            case rhi::PixelFormat::Rgba8:
                return PixelFormatEnum::RGBA8;

            case rhi::PixelFormat::Rgba16F:
                return PixelFormatEnum::RGBA16F;

            case rhi::PixelFormat::Rgba32F:
                return PixelFormatEnum::RGBA32F;

            case rhi::PixelFormat::Depth32:
                return PixelFormatEnum::Depth;

            case rhi::PixelFormat::Depth24Stencil8:
                return PixelFormatEnum::DepthStencil;

            default:
                return PixelFormatEnum::RGBA8;
        }
    }

    rhi::PixelFormat to_rhi_pixel_format(const PixelFormatEnum format) {
        switch(format) {
            case PixelFormatEnum::RGBA8:
                return rhi::PixelFormat::Rgba8;

            case PixelFormatEnum::RGBA16F:
                return rhi::PixelFormat::Rgba16F;

            case PixelFormatEnum::RGBA32F:
                return rhi::PixelFormat::Rgba32F;

            case PixelFormatEnum::Depth:
                return rhi::PixelFormat::Depth32;

            case PixelFormatEnum::DepthStencil:
                return rhi::PixelFormat::Depth24Stencil8;

            default:
                return rhi::PixelFormat::Rgba8;
        }
    }
} // namespace nova::renderer::shaderpack
