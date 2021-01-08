#include "nova_renderer/renderpack_data.hpp"

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

namespace nova::renderer::renderpack {
    RX_LOG("RenderpackData", logger);

    bool TextureFormat::operator==(const TextureFormat& other) const {
        return pixel_format == other.pixel_format && dimension_type == other.dimension_type && width == other.width &&
               height == other.height;
    }

    bool TextureFormat::operator!=(const TextureFormat& other) const { return !(*this == other); }

    TextureFormat TextureFormat::from_json(const rx::json& json) {
        TextureFormat format = {};

        format.pixel_format = get_json_value(json, "pixelFormat", rhi::PixelFormat::Rgba8, pixel_format_enum_from_json);
        format.dimension_type = get_json_value<TextureDimensionType>(json,
                                                                     "dimensionType",
                                                                     TextureDimensionType::ScreenRelative,
                                                                     texture_dimension_type_enum_from_json);
        format.width = get_json_value<float>(json, "width", 0);
        format.height = get_json_value<float>(json, "height", 0);

        return format;
    }

    TextureCreateInfo TextureCreateInfo::from_json(const rx::json& json) {
        TextureCreateInfo info = {};

        FILL_REQUIRED_FIELD(info.name, get_json_opt<std::string>(json, "name"));
        FILL_REQUIRED_FIELD(info.format, get_json_opt<TextureFormat>(json, "format"));

        return info;
    }

    RenderpackResourcesData RenderpackResourcesData::from_json(const rx::json& json) {
        RenderpackResourcesData data;
        data.render_targets = get_json_array<TextureCreateInfo>(json, "textures");
        data.samplers = get_json_array<SamplerCreateInfo>(json, "samplers");

        // TODO: buffers
        // TODO: arbitrary images

        return data;
    }

    bool TextureAttachmentInfo::operator==(const TextureAttachmentInfo& other) const { return other.name == name; }

    TextureAttachmentInfo TextureAttachmentInfo::from_json(const rx::json& json) {
        TextureAttachmentInfo info = {};

        FILL_REQUIRED_FIELD(info.name, get_json_opt<std::string>(json, "name"));
        info.clear = get_json_value(json, "clear", false);

        return info;
    }

    RenderPassCreateInfo RenderPassCreateInfo::from_json(const rx::json& json) {
        RenderPassCreateInfo info = {};

        info.texture_inputs = get_json_array<std::string>(json, "textureInputs");
        info.texture_outputs = get_json_array<TextureAttachmentInfo>(json, "textureOutputs");
        info.depth_texture = get_json_opt<TextureAttachmentInfo>(json, "depthTexture");

        info.input_buffers = get_json_array<std::string>(json, "inputBuffers");
        info.output_buffers = get_json_array<std::string>(json, "outputBuffers");

        info.name = get_json_value<std::string>(json, "name", "<NAME_MISSING>");

        return info;
    }

    RendergraphData RendergraphData::from_json(const rx::json& json) {
        RendergraphData data;

        data.passes = get_json_array<RenderPassCreateInfo>(json, "passes");
        data.builtin_passes = get_json_array<std::string>(json, "builtinPasses");

        return data;
    }

    SamplerCreateInfo SamplerCreateInfo::from_json(const rx::json& json) {
        SamplerCreateInfo info = {};

        info.filter = get_json_value(json, "filter", TextureFilter::Point, texture_filter_enum_from_json);
        info.wrap_mode = get_json_value(json, "wrapMode", WrapMode::Clamp, wrap_mode_enum_from_json);

        return info;
    }

    StencilOpState StencilOpState::from_json(const rx::json& json) {
        StencilOpState state = {};

        FILL_REQUIRED_FIELD(state.fail_op, get_json_opt<RPStencilOp>(json, "failOp", stencil_op_enum_from_json));
        FILL_REQUIRED_FIELD(state.pass_op, get_json_opt<RPStencilOp>(json, "passOp", stencil_op_enum_from_json));
        FILL_REQUIRED_FIELD(state.depth_fail_op, get_json_opt<RPStencilOp>(json, "depthFailOp", stencil_op_enum_from_json));
        FILL_REQUIRED_FIELD(state.compare_op, get_json_opt<RPCompareOp>(json, "compareOp", compare_op_enum_from_json));
        FILL_REQUIRED_FIELD(state.compare_mask, get_json_opt<uint32_t>(json, "compareMask"));
        FILL_REQUIRED_FIELD(state.write_mask, get_json_opt<uint32_t>(json, "writeMask"));

        return state;
    }

    PipelineData PipelineData::from_json(const rx::json& json) {
        PipelineData pipeline = {};

        FILL_REQUIRED_FIELD(pipeline.name, get_json_opt<std::string>(json, "name"));
        FILL_REQUIRED_FIELD(pipeline.pass, get_json_opt<std::string>(json, "pass"));
        pipeline.parent_name = get_json_value(json, "parent", "");

        pipeline.defines = get_json_array<std::string>(json, "defined");

        pipeline.states = get_json_array<RasterizerState>(json, "states", state_enum_from_json);
        pipeline.front_face = get_json_opt<StencilOpState>(json, "frontFace");
        pipeline.back_face = get_json_opt<StencilOpState>(json, "backFace");
        pipeline.fallback = get_json_value<std::string>(json, "fallback", {});
        pipeline.depth_bias = get_json_value<float>(json, "depthBias", 0);
        pipeline.slope_scaled_depth_bias = get_json_value<float>(json, "slopeScaledDepthBias", 0);
        pipeline.stencil_ref = get_json_value<uint32_t>(json, "stencilRef", 0);
        pipeline.stencil_read_mask = get_json_value<uint32_t>(json, "stencilReadMask", 0);
        pipeline.stencil_write_mask = get_json_value<uint32_t>(json, "stencilWriteMask", 0);
        pipeline.msaa_support = get_json_value<MsaaSupport>(json, "msaaSupport", MsaaSupport::None, msaa_support_enum_from_json);
        pipeline.primitive_mode = get_json_value<RPPrimitiveTopology>(json,
                                                                      "primitiveMode",
                                                                      RPPrimitiveTopology::Triangles,
                                                                      primitive_topology_enum_from_json);
        pipeline.source_color_blend_factor = get_json_value<RPBlendFactor>(json,
                                                                           "sourceBlendFactor",
                                                                           RPBlendFactor::One,
                                                                           blend_factor_enum_from_json);
        pipeline.destination_color_blend_factor = get_json_value<RPBlendFactor>(json,
                                                                                "destBlendFactor",
                                                                                RPBlendFactor::Zero,
                                                                                blend_factor_enum_from_json);
        pipeline.source_alpha_blend_factor = get_json_value<RPBlendFactor>(json,
                                                                           "alphaSrc",
                                                                           RPBlendFactor::One,
                                                                           blend_factor_enum_from_json);
        pipeline.destination_alpha_blend_factor = get_json_value<RPBlendFactor>(json,
                                                                                "alphaDest",
                                                                                RPBlendFactor::Zero,
                                                                                blend_factor_enum_from_json);
        pipeline.depth_func = get_json_value<RPCompareOp>(json, "depthFunc", RPCompareOp::Less, compare_op_enum_from_json);
        pipeline.render_queue = get_json_value<RenderQueue>(json, "renderQueue", RenderQueue::Opaque, render_queue_enum_from_json);

        pipeline.scissor_mode = get_json_value<ScissorTestMode>(json, "scissorMode", ScissorTestMode::Off, scissor_test_mode_from_json);

        pipeline.vertex_shader.filename = get_json_value<std::string>(json, "vertexShader", "<NAME_MISSING>");

        const auto geometry_shader_name = get_json_opt<std::string>(json, "geometryShader");
        if(geometry_shader_name) {
            pipeline.geometry_shader = RenderpackShaderSource{};
            pipeline.geometry_shader->filename = *geometry_shader_name;
        }

        const auto tess_control_shader_name = get_json_opt<std::string>(json, "tessellationControlShader");
        if(tess_control_shader_name) {
            pipeline.tessellation_control_shader = RenderpackShaderSource{};
            pipeline.tessellation_control_shader->filename = *tess_control_shader_name;
        }

        const auto tess_eval_shader_name = get_json_opt<std::string>(json, "tessellationEvalShader");
        if(tess_eval_shader_name) {
            pipeline.tessellation_evaluation_shader = RenderpackShaderSource{};
            pipeline.tessellation_evaluation_shader->filename = *tess_eval_shader_name;
        }

        const auto fragment_shader_name = get_json_opt<std::string>(json, "fragmentShader");
        if(fragment_shader_name) {
            pipeline.fragment_shader = RenderpackShaderSource{};
            pipeline.fragment_shader->filename = *fragment_shader_name;
        }

        return pipeline;
    }

    glm::uvec2 TextureFormat::get_size_in_pixels(const glm::uvec2& screen_size) const {
        float pixel_width = width;
        float pixel_height = height;

        if(dimension_type == TextureDimensionType::ScreenRelative) {
            pixel_width *= static_cast<float>(screen_size.x);
            pixel_height *= static_cast<float>(screen_size.y);
        }

        return {std::round(pixel_width), std::round(pixel_height)};
    }

    rx::optional<std::unordered_map<std::string, std::string>> map_from_json_object(const rx::json& json) {
        std::unordered_map<std::string, std::string> map;

        json.each([&](const rx::json& elem) {
            std::string shader_variable;
            FILL_REQUIRED_FIELD(shader_variable, get_json_opt<std::string>(elem, "variable"));

            std::string resource_name;
            FILL_REQUIRED_FIELD(resource_name, get_json_opt<std::string>(elem, "resource"));

            map.insert(shader_variable, resource_name);
        });

        return map;
    }

    MaterialPass MaterialPass::from_json(const rx::json& json) {
        MaterialPass pass = {};

        FILL_REQUIRED_FIELD(pass.name, get_json_opt<std::string>(json, "name"));
        FILL_REQUIRED_FIELD(pass.pipeline, get_json_opt<std::string>(json, "pipeline"));

        const auto val = get_json_opt<std::unordered_map<std::string, std::string>>(json, "bindings", map_from_json_object);
        if(val) {
            pass.bindings = *val;
        }

        // FILL_REQUIRED_FIELD(pass.bindings, get_json_opt<std::unordered_map<std::string, std::string>>(json, "bindings", map_from_json_object));

        return pass;
    }

    MaterialData MaterialData::from_json(const rx::json& json) {
        MaterialData data = {};

        FILL_REQUIRED_FIELD(data.name, get_json_opt<std::string>(json, "name"));
        data.passes = get_json_array<MaterialPass>(json, "passes");
        FILL_REQUIRED_FIELD(data.geometry_filter, get_json_opt<std::string>(json, "filter"));

        return data;
    }

    rhi::PixelFormat pixel_format_enum_from_string(const std::string& str) {
        if(str == "RGBA8") {
            return rhi::PixelFormat::Rgba8;
        }
        if(str == "RGBA16F") {
            return rhi::PixelFormat::Rgba16F;
        }
        if(str == "RGBA32F") {
            return rhi::PixelFormat::Rgba32F;
        }
        if(str == "Depth") {
            return rhi::PixelFormat::Depth32;
        }
        if(str == "DepthStencil") {
            return rhi::PixelFormat::Depth24Stencil8;
        }

        logger->error("Unsupported pixel format %s", str);
        return {};
    }

    TextureDimensionType texture_dimension_type_enum_from_string(const std::string& str) {
        if(str == "ScreenRelative") {
            return TextureDimensionType ::ScreenRelative;
        }
        if(str == "Absolute") {
            return TextureDimensionType::Absolute;
        }

        logger->error("Unsupported texture dimension type %s", str);
        return {};
    }

    TextureFilter texture_filter_enum_from_string(const std::string& str) {
        if(str == "TexelAA") {
            return TextureFilter::TexelAA;
        }
        if(str == "Bilinear") {
            return TextureFilter::Bilinear;
        }
        if(str == "Point") {
            return TextureFilter::Point;
        }

        logger->error("Unsupported texture filter %s", str);
        return {};
    }

    WrapMode wrap_mode_enum_from_string(const std::string& str) {
        if(str == "Repeat") {
            return WrapMode::Repeat;
        }
        if(str == "Clamp") {
            return WrapMode::Clamp;
        }

        logger->error("Unsupported wrap mode %s", str);
        return {};
    }

    RPStencilOp stencil_op_enum_from_string(const std::string& str) {
        if(str == "Keep") {
            return RPStencilOp::Keep;
        }
        if(str == "Zero") {
            return RPStencilOp::Zero;
        }
        if(str == "Replace") {
            return RPStencilOp::Replace;
        }
        if(str == "Incr") {
            return RPStencilOp::Increment;
        }
        if(str == "IncrWrap") {
            return RPStencilOp::IncrementAndWrap;
        }
        if(str == "Decr") {
            return RPStencilOp::Decrement;
        }
        if(str == "DecrWrap") {
            return RPStencilOp::DecrementAndWrap;
        }
        if(str == "Invert") {
            return RPStencilOp::Invert;
        }

        logger->error("Unsupported stencil op %s", str);
        return {};
    }

    RPCompareOp compare_op_enum_from_string(const std::string& str) {
        if(str == "Never") {
            return RPCompareOp::Never;
        }
        if(str == "Less") {
            return RPCompareOp::Less;
        }
        if(str == "LessEqual") {
            return RPCompareOp::LessEqual;
        }
        if(str == "Greater") {
            return RPCompareOp::Greater;
        }
        if(str == "GreaterEqual") {
            return RPCompareOp::GreaterEqual;
        }
        if(str == "Equal") {
            return RPCompareOp::Equal;
        }
        if(str == "NotEqual") {
            return RPCompareOp::NotEqual;
        }
        if(str == "Always") {
            return RPCompareOp::Always;
        }

        logger->error("Unsupported compare op ", str);
        return {};
    }

    MsaaSupport msaa_support_enum_from_string(const std::string& str) {
        if(str == "MSAA") {
            return MsaaSupport::MSAA;
        }
        if(str == "Both") {
            return MsaaSupport::Both;
        }
        if(str == "None") {
            return MsaaSupport::None;
        }

        logger->error("Unsupported antialiasing mode %s", str);
        return {};
    }

    RPPrimitiveTopology primitive_topology_enum_from_string(const std::string& str) {
        if(str == "Triangles") {
            return RPPrimitiveTopology::Triangles;
        }
        if(str == "Lines") {
            return RPPrimitiveTopology::Lines;
        }

        logger->error("Unsupported primitive mode %s", str);
        return {};
    }

    RPBlendFactor blend_factor_enum_from_string(const std::string& str) {
        if(str == "One") {
            return RPBlendFactor::One;
        }
        if(str == "Zero") {
            return RPBlendFactor::Zero;
        }
        if(str == "SrcColor") {
            return RPBlendFactor::SrcColor;
        }
        if(str == "DstColor") {
            return RPBlendFactor::DstColor;
        }
        if(str == "OneMinusSrcColor") {
            return RPBlendFactor::OneMinusSrcColor;
        }
        if(str == "OneMinusDstColor") {
            return RPBlendFactor::OneMinusDstColor;
        }
        if(str == "SrcAlpha") {
            return RPBlendFactor::SrcAlpha;
        }
        if(str == "DstAlpha") {
            return RPBlendFactor::DstAlpha;
        }
        if(str == "OneMinusSrcAlpha") {
            return RPBlendFactor::OneMinusSrcAlpha;
        }
        if(str == "OneMinusDstAlpha") {
            return RPBlendFactor::OneMinusDstAlpha;
        }

        logger->error("Unsupported blend factor %s", str);
        return {};
    }

    RenderQueue render_queue_enum_from_string(const std::string& str) {
        if(str == "Transparent") {
            return RenderQueue::Transparent;
        }
        if(str == "Opaque") {
            return RenderQueue::Opaque;
        }
        if(str == "Cutout") {
            return RenderQueue::Cutout;
        }

        logger->error("Unsupported render queue %s", str);
        return {};
    }

    ScissorTestMode scissor_test_mode_from_string(const std::string& str) {
        if(str == "Off") {
            return ScissorTestMode::Off;

        } else if(str == "StaticScissorRect") {
            return ScissorTestMode::StaticScissorRect;

        } else if(str == "DynamicScissorRect") {
            return ScissorTestMode::DynamicScissorRect;
        }

        logger->error("Unsupported scissor mode %s", str);
        return {};
    }

    RasterizerState state_enum_from_string(const std::string& str) {
        if(str == "Blending") {
            return RasterizerState::Blending;
        }
        if(str == "InvertCulling") {
            return RasterizerState::InvertCulling;
        }
        if(str == "DisableCulling") {
            return RasterizerState::DisableCulling;
        }
        if(str == "DisableDepthWrite") {
            return RasterizerState::DisableDepthWrite;
        }
        if(str == "DisableDepthTest") {
            return RasterizerState::DisableDepthTest;
        }
        if(str == "EnableStencilTest") {
            return RasterizerState::EnableStencilTest;
        }
        if(str == "StencilWrite") {
            return RasterizerState::StencilWrite;
        }
        if(str == "DisableColorWrite") {
            return RasterizerState::DisableColorWrite;
        }
        if(str == "EnableAlphaToCoverage") {
            return RasterizerState::EnableAlphaToCoverage;
        }
        if(str == "DisableAlphaWrite") {
            return RasterizerState::DisableAlphaWrite;
        }

        logger->error("Unsupported state enum %s", str);
        return {};
    }

    rhi::PixelFormat pixel_format_enum_from_json(const rx::json& j) { return pixel_format_enum_from_string(j.as_string()); }

    TextureDimensionType texture_dimension_type_enum_from_json(const rx::json& j) {
        return texture_dimension_type_enum_from_string(j.as_string());
    }

    TextureFilter texture_filter_enum_from_json(const rx::json& j) { return texture_filter_enum_from_string(j.as_string()); }

    WrapMode wrap_mode_enum_from_json(const rx::json& j) { return wrap_mode_enum_from_string(j.as_string()); }

    RPStencilOp stencil_op_enum_from_json(const rx::json& j) { return stencil_op_enum_from_string(j.as_string()); }

    RPCompareOp compare_op_enum_from_json(const rx::json& j) { return compare_op_enum_from_string(j.as_string()); }

    MsaaSupport msaa_support_enum_from_json(const rx::json& j) { return msaa_support_enum_from_string(j.as_string()); }

    RPPrimitiveTopology primitive_topology_enum_from_json(const rx::json& j) { return primitive_topology_enum_from_string(j.as_string()); }

    RPBlendFactor blend_factor_enum_from_json(const rx::json& j) { return blend_factor_enum_from_string(j.as_string()); }

    RenderQueue render_queue_enum_from_json(const rx::json& j) { return render_queue_enum_from_string(j.as_string()); }

    ScissorTestMode scissor_test_mode_from_json(const rx::json& j) { return scissor_test_mode_from_string(j.as_string()); }

    RasterizerState state_enum_from_json(const rx::json& j) { return state_enum_from_string(j.as_string()); }

    std::string to_string(const rhi::PixelFormat val) {
        switch(val) {
            case rhi::PixelFormat::Rgba8:
                return "RGBA8";

            case rhi::PixelFormat::Rgba16F:
                return "RGBA16F";

            case rhi::PixelFormat::Rgba32F:
                return "RGBA32F";

            case rhi::PixelFormat::Depth32:
                return "Depth";

            case rhi::PixelFormat::Depth24Stencil8:
                return "DepthStencil";
        }

        return "Unknown value";
    }

    std::string to_string(const TextureDimensionType val) {
        switch(val) {
            case TextureDimensionType::ScreenRelative:
                return "ScreenRelative";

            case TextureDimensionType::Absolute:
                return "Absolute";
        }

        return "Unknown value";
    }

    std::string to_string(const TextureFilter val) {
        switch(val) {
            case TextureFilter::TexelAA:
                return "TexelAA";

            case TextureFilter::Bilinear:
                return "Bilinear";

            case TextureFilter::Point:
                return "Point";
        }

        return "Unknown value";
    }

    std::string to_string(const WrapMode val) {
        switch(val) {
            case WrapMode::Repeat:
                return "Repeat";

            case WrapMode::Clamp:
                return "Clamp";
        }

        return "Unknown value";
    }

    std::string to_string(const RPStencilOp val) {
        switch(val) {
            case RPStencilOp::Keep:
                return "Keep";

            case RPStencilOp::Zero:
                return "Zero";

            case RPStencilOp::Replace:
                return "Replace";

            case RPStencilOp::Increment:
                return "Incr";

            case RPStencilOp::IncrementAndWrap:
                return "IncrWrap";

            case RPStencilOp::Decrement:
                return "Decr";

            case RPStencilOp::DecrementAndWrap:
                return "DecrWrap";

            case RPStencilOp::Invert:
                return "Invert";
        }

        return "Unknown value";
    }

    std::string to_string(const RPCompareOp val) {
        switch(val) {
            case RPCompareOp::Never:
                return "Never";

            case RPCompareOp::Less:
                return "Less";

            case RPCompareOp::LessEqual:
                return "LessEqual";

            case RPCompareOp::Greater:
                return "Greater";

            case RPCompareOp::GreaterEqual:
                return "GreaterEqual";

            case RPCompareOp::Equal:
                return "Equal";

            case RPCompareOp::NotEqual:
                return "NotEqual";

            case RPCompareOp::Always:
                return "Always";
        }

        return "Unknown value";
    }

    std::string to_string(const MsaaSupport val) {
        switch(val) {
            case MsaaSupport::MSAA:
                return "MSAA";

            case MsaaSupport::Both:
                return "Both";

            case MsaaSupport::None:
                return "None";
        }

        return "Unknown value";
    }

    std::string to_string(const RPPrimitiveTopology val) {
        switch(val) {
            case RPPrimitiveTopology::Triangles:
                return "Triangles";

            case RPPrimitiveTopology::Lines:
                return "Lines";
        }

        return "Unknown value";
    }

    std::string to_string(const RPBlendFactor val) {
        switch(val) {
            case RPBlendFactor::One:
                return "One";

            case RPBlendFactor::Zero:
                return "Zero";

            case RPBlendFactor::SrcColor:
                return "SrcColor";

            case RPBlendFactor::DstColor:
                return "DstColor";

            case RPBlendFactor::OneMinusSrcColor:
                return "OneMinusSrcColor";

            case RPBlendFactor::OneMinusDstColor:
                return "OneMinusDstColor";

            case RPBlendFactor::SrcAlpha:
                return "SrcAlpha";

            case RPBlendFactor::DstAlpha:
                return "DstAlpha";

            case RPBlendFactor::OneMinusSrcAlpha:
                return "OneMinusSrcAlpha";

            case RPBlendFactor::OneMinusDstAlpha:
                return "OneMinusDstAlpha";
        }

        return "Unknown value";
    }

    std::string to_string(const RenderQueue val) {
        switch(val) {
            case RenderQueue::Transparent:
                return "Transparent";

            case RenderQueue::Opaque:
                return "Opaque";

            case RenderQueue::Cutout:
                return "Cutout";
        }

        return "Unknown value";
    }

    std::string to_string(const RasterizerState val) {
        switch(val) {
            case RasterizerState::Blending:
                return "Blending";

            case RasterizerState::InvertCulling:
                return "InvertCulling";

            case RasterizerState::DisableCulling:
                return "DisableCulling";

            case RasterizerState::DisableDepthWrite:
                return "DisableDepthWrite";

            case RasterizerState::DisableDepthTest:
                return "DisableDepthTest";

            case RasterizerState::EnableStencilTest:
                return "EnableStencilTest";

            case RasterizerState::StencilWrite:
                return "StencilWrite";

            case RasterizerState::DisableColorWrite:
                return "DisableColorWrite";

            case RasterizerState::EnableAlphaToCoverage:
                return "EnableAlphaToCoverage";

            case RasterizerState::DisableAlphaWrite:
                return "DisableAlphaWrite";
        }

        return "Unknown value";
    }

    uint32_t pixel_format_to_pixel_width(const rhi::PixelFormat format) {
        switch(format) {
            case rhi::PixelFormat::Rgba8:
                return 4 * 8;

            case rhi::PixelFormat::Rgba16F:
                return 4 * 16;

            case rhi::PixelFormat::Rgba32F:
                return 4 * 32;

            case rhi::PixelFormat::Depth32:
                return 32;

            case rhi::PixelFormat::Depth24Stencil8:
                return 32;

            default:
                return 32;
        }
    }
} // namespace nova::renderer::renderpack
