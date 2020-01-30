#include "json_interop.hpp"

#include "../json_utils.hpp"

namespace nova::renderer::shaderpack {
    void from_json(const nlohmann::json& j, TextureFormat& format) {
        format.pixel_format = get_json_value<PixelFormatEnum>(j, "pixelFormat", PixelFormatEnum::RGBA8, pixel_format_enum_from_string);
        format.dimension_type = get_json_value<TextureDimensionTypeEnum>(j,
                                                                         "dimensionType",
                                                                         TextureDimensionTypeEnum::ScreenRelative,
                                                                         texture_dimension_type_enum_from_string);

        format.width = get_json_value<float>(j, "width", 0);
        format.height = get_json_value<float>(j, "height", 0);
    }

    void from_json(const nlohmann::json& j, TextureCreateInfo& tex) {
        tex.name = *get_json_value<rx::string>(j, "name");
        tex.format = *get_json_value<TextureFormat>(j, "format");
    }

    void from_json(const nlohmann::json& j, SamplerCreateInfo& sampler) {
        sampler.filter = get_json_value<TextureFilterEnum>(j, "filter", TextureFilterEnum::Point, texture_filter_enum_from_string);
        sampler.wrap_mode = get_json_value<WrapModeEnum>(j, "wrapMode", WrapModeEnum::Clamp, wrap_mode_enum_from_string);
    }

    void from_json(const nlohmann::json& j, ShaderpackResourcesData& res) {
        res.render_targets = get_json_array<TextureCreateInfo>(j, "textures");
        res.samplers = get_json_array<SamplerCreateInfo>(j, "samplers");
    }

    void from_json(const nlohmann::json& j, RenderPassCreateInfo& pass) {
        // Something something string bad
        const auto& std_vec = get_json_array<std::string>(j, "textureInputs").each_fwd([&](const std::string& str) {
            pass.texture_inputs.emplace_back(str.c_str());
        });

        pass.texture_outputs = get_json_array<TextureAttachmentInfo>(j, "textureOutputs");
        pass.depth_texture = get_json_value<TextureAttachmentInfo>(j, "depthTexture");

        get_json_array<std::string>(j, "inputBuffers").each_fwd([&](const std::string& str) {
            pass.input_buffers.emplace_back(str.c_str());
        });
        get_json_array<std::string>(j, "outputBuffers").each_fwd([&](const std::string& str) {
            pass.output_buffers.emplace_back(str.c_str());
        });

        pass.name = get_json_value<std::string>(j, "name", std::string{"<NAME_MISSING>"}).c_str();
    }

    void from_json(const nlohmann::json& j, StencilOpState& stencil_op) {
        stencil_op.fail_op = get_json_value<StencilOpEnum>(j, "failOp", StencilOpEnum::Keep, stencil_op_enum_from_string);
        stencil_op.pass_op = get_json_value<StencilOpEnum>(j, "passOp", StencilOpEnum::Keep, stencil_op_enum_from_string);
        stencil_op.depth_fail_op = get_json_value<StencilOpEnum>(j, "depthFailOp", StencilOpEnum::Keep, stencil_op_enum_from_string);
        stencil_op.compare_op = get_json_value<CompareOpEnum>(j, "compareOp", CompareOpEnum::Equal, compare_op_enum_from_string);
        stencil_op.compare_mask = get_json_value<uint32_t>(j, "compareMask", 0);
        stencil_op.write_mask = get_json_value<uint32_t>(j, "writeMask", 0);
    }

    void from_json(const nlohmann::json& j, PipelineCreateInfo& pipeline) {
        pipeline.name = get_json_value<std::string>(j, "name")->c_str();
        pipeline.parent_name = get_json_value<std::string>(j, "parent", std::string{}).c_str();
        pipeline.pass = get_json_value<std::string>(j, "pass")->c_str();

        get_json_array<std::string>(j, "defines").each_fwd([&](const std::string& str) { pipeline.defines.emplace_back(str.c_str()); });

        pipeline.states = get_json_array<StateEnum>(j, "states", state_enum_from_string);
        pipeline.front_face = get_json_value<StencilOpState>(j, "frontFace");
        pipeline.back_face = get_json_value<StencilOpState>(j, "backFace");
        pipeline.fallback = get_json_value<std::string>(j, "fallback", std::string{}).c_str();
        pipeline.depth_bias = get_json_value<float>(j, "depthBias", 0);
        pipeline.slope_scaled_depth_bias = get_json_value<float>(j, "slopeScaledDepthBias", 0);
        pipeline.stencil_ref = get_json_value<uint32_t>(j, "stencilRef", 0);
        pipeline.stencil_read_mask = get_json_value<uint32_t>(j, "stencilReadMask", 0);
        pipeline.stencil_write_mask = get_json_value<uint32_t>(j, "stencilWriteMask", 0);
        pipeline.msaa_support = get_json_value<MsaaSupportEnum>(j, "msaaSupport", MsaaSupportEnum::None, msaa_support_enum_from_string);
        pipeline.primitive_mode = get_json_value<PrimitiveTopologyEnum>(j,
                                                                        "primitiveMode",
                                                                        PrimitiveTopologyEnum::Triangles,
                                                                        primitive_topology_enum_from_string);
        pipeline.source_color_blend_factor = get_json_value<BlendFactorEnum>(j,
                                                                             "sourceBlendFactor",
                                                                             BlendFactorEnum::One,
                                                                             blend_factor_enum_from_string);
        pipeline.destination_color_blend_factor = get_json_value<BlendFactorEnum>(j,
                                                                                  "destBlendFactor",
                                                                                  BlendFactorEnum::Zero,
                                                                                  blend_factor_enum_from_string);
        pipeline.source_alpha_blend_factor = get_json_value<BlendFactorEnum>(j,
                                                                             "alphaSrc",
                                                                             BlendFactorEnum::One,
                                                                             blend_factor_enum_from_string);
        pipeline.destination_alpha_blend_factor = get_json_value<BlendFactorEnum>(j,
                                                                                  "alphaDest",
                                                                                  BlendFactorEnum::Zero,
                                                                                  blend_factor_enum_from_string);
        pipeline.depth_func = get_json_value<CompareOpEnum>(j, "depthFunc", CompareOpEnum::Less, compare_op_enum_from_string);
        pipeline.render_queue = get_json_value<RenderQueueEnum>(j, "renderQueue", RenderQueueEnum::Opaque, render_queue_enum_from_string);

        pipeline.vertex_shader.filename = get_json_value<std::string>(j, "vertexShader", std::string{"<NAME_MISSING>"}).c_str();

        rx::optional<std::string> geometry_shader_name = get_json_value<std::string>(j, "geometryShader");
        if(geometry_shader_name) {
            pipeline.geometry_shader = rx::optional<ShaderSource>();
            pipeline.geometry_shader->filename = geometry_shader_name->c_str();
        }

        rx::optional<std::string> tess_control_shader_name = get_json_value<std::string>(j, "tessellationControlShader");
        if(tess_control_shader_name) {
            pipeline.tessellation_control_shader = rx::optional<ShaderSource>();
            pipeline.tessellation_control_shader->filename = tess_control_shader_name->c_str();
        }

        rx::optional<std::string> tess_eval_shader_name = get_json_value<std::string>(j, "tessellationEvalShader");
        if(tess_eval_shader_name) {
            pipeline.tessellation_evaluation_shader = rx::optional<ShaderSource>();
            pipeline.tessellation_evaluation_shader->filename = tess_eval_shader_name->c_str();
        }

        rx::optional<std::string> fragment_shader_name = get_json_value<std::string>(j, "fragmentShader");
        if(fragment_shader_name) {
            pipeline.fragment_shader = rx::optional<ShaderSource>();
            pipeline.fragment_shader->filename = fragment_shader_name->c_str();
        }
    }

    void from_json(const nlohmann::json& j, MaterialPass& pass) {
        pass.name = *get_json_value<rx::string>(j, "name");
        pass.pipeline = *get_json_value<rx::string>(j, "pipeline");

        // std allowed for JSON interop
        const auto& bindings_map = *get_json_value<std::unordered_map<std::string, std::string>>(j, "bindings");
        for(const auto& [binding_name, bound_resource] : bindings_map) {
            pass.bindings.insert(binding_name.c_str(), bound_resource.c_str());
        }
    }

    void from_json(const nlohmann::json& j, MaterialData& mat) {
        mat.name = *get_json_value<rx::string>(j, "name");
        mat.passes = get_json_array<MaterialPass>(j, "passes");
        mat.geometry_filter = *get_json_value<rx::string>(j, "filter");

        mat.passes.each_fwd([&](MaterialPass& pass) { pass.material_name = mat.name; });
    }

    void from_json(const nlohmann::json& j, TextureAttachmentInfo& tex) {
        tex.name = *get_json_value<rx::string>(j, "name");
        tex.clear = get_json_value<bool>(j, "clear", false);
    }

    void from_json(const nlohmann::json& j, rx::vector<RenderPassCreateInfo>& passes) {
        for(const auto& node : j) {
            passes.push_back(node.get<RenderPassCreateInfo>());
        }
    }

    void from_json(const nlohmann::json& j, RendergraphData& data) {
        data.passes = get_json_array<RenderPassCreateInfo>(j, "passes");
        get_json_array<std::string>(j, "builtin_passes").each_fwd([&](const std::string& str) {
            data.builtin_passes.emplace_back(str.c_str());
        });
    }
} // namespace nova::renderer::shaderpack
