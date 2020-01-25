/*!
 * \author ddubois
 * \date 21-Aug-18.
 */

#include "nova_renderer/loading/shaderpack_loading.hpp"

#define ENABLE_HLSL
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include "nova_renderer/constants.hpp"
#include "nova_renderer/filesystem/folder_accessor.hpp"
#include "nova_renderer/filesystem/virtual_filesystem.hpp"

#include "../../tasks/task_scheduler.hpp"
#include "../json_utils.hpp"
#include "json_interop.hpp"
#include "render_graph_builder.hpp"
#include "shaderpack_validator.hpp"

namespace nova::renderer::shaderpack {
    using namespace filesystem;

    // Removed from the GLSLang version we're using
    // TODO: Copy and fill in with values from the RHI so we don't accidentally limit a shader
    const TBuiltInResource DEFAULT_BUILT_IN_RESOURCE = {
        /* .MaxLights = */ 32,
        /* .MaxClipPlanes = */ 6,
        /* .MaxTextureUnits = */ 32,
        /* .MaxTextureCoords = */ 32,
        /* .MaxVertexAttribs = */ 64,
        /* .MaxVertexUniformComponents = */ 4096,
        /* .MaxVaryingFloats = */ 64,
        /* .MaxVertexTextureImageUnits = */ 32,
        /* .MaxCombinedTextureImageUnits = */ 80,
        /* .MaxTextureImageUnits = */ 32,
        /* .MaxFragmentUniformComponents = */ 4096,
        /* .MaxDrawBuffers = */ 32,
        /* .MaxVertexUniformVectors = */ 128,
        /* .MaxVaryingVectors = */ 8,
        /* .MaxFragmentUniformVectors = */ 16,
        /* .MaxVertexOutputVectors = */ 16,
        /* .MaxFragmentInputVectors = */ 15,
        /* .MinProgramTexelOffset = */ -8,
        /* .MaxProgramTexelOffset = */ 7,
        /* .MaxClipDistances = */ 8,
        /* .MaxComputeWorkGroupCountX = */ 65535,
        /* .MaxComputeWorkGroupCountY = */ 65535,
        /* .MaxComputeWorkGroupCountZ = */ 65535,
        /* .MaxComputeWorkGroupSizeX = */ 1024,
        /* .MaxComputeWorkGroupSizeY = */ 1024,
        /* .MaxComputeWorkGroupSizeZ = */ 64,
        /* .MaxComputeUniformComponents = */ 1024,
        /* .MaxComputeTextureImageUnits = */ 16,
        /* .MaxComputeImageUniforms = */ 8,
        /* .MaxComputeAtomicCounters = */ 8,
        /* .MaxComputeAtomicCounterBuffers = */ 1,
        /* .MaxVaryingComponents = */ 60,
        /* .MaxVertexOutputComponents = */ 64,
        /* .MaxGeometryInputComponents = */ 64,
        /* .MaxGeometryOutputComponents = */ 128,
        /* .MaxFragmentInputComponents = */ 128,
        /* .MaxImageUnits = */ 8,
        /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
        /* .MaxCombinedShaderOutputResources = */ 8,
        /* .MaxImageSamples = */ 0,
        /* .MaxVertexImageUniforms = */ 0,
        /* .MaxTessControlImageUniforms = */ 0,
        /* .MaxTessEvaluationImageUniforms = */ 0,
        /* .MaxGeometryImageUniforms = */ 0,
        /* .MaxFragmentImageUniforms = */ 8,
        /* .MaxCombinedImageUniforms = */ 8,
        /* .MaxGeometryTextureImageUnits = */ 16,
        /* .MaxGeometryOutputVertices = */ 256,
        /* .MaxGeometryTotalOutputComponents = */ 1024,
        /* .MaxGeometryUniformComponents = */ 1024,
        /* .MaxGeometryVaryingComponents = */ 64,
        /* .MaxTessControlInputComponents = */ 128,
        /* .MaxTessControlOutputComponents = */ 128,
        /* .MaxTessControlTextureImageUnits = */ 16,
        /* .MaxTessControlUniformComponents = */ 1024,
        /* .MaxTessControlTotalOutputComponents = */ 4096,
        /* .MaxTessEvaluationInputComponents = */ 128,
        /* .MaxTessEvaluationOutputComponents = */ 128,
        /* .MaxTessEvaluationTextureImageUnits = */ 16,
        /* .MaxTessEvaluationUniformComponents = */ 1024,
        /* .MaxTessPatchComponents = */ 120,
        /* .MaxPatchVertices = */ 32,
        /* .MaxTessGenLevel = */ 64,
        /* .MaxViewports = */ 16,
        /* .MaxVertexAtomicCounters = */ 0,
        /* .MaxTessControlAtomicCounters = */ 0,
        /* .MaxTessEvaluationAtomicCounters = */ 0,
        /* .MaxGeometryAtomicCounters = */ 0,
        /* .MaxFragmentAtomicCounters = */ 8,
        /* .MaxCombinedAtomicCounters = */ 8,
        /* .MaxAtomicCounterBindings = */ 1,
        /* .MaxVertexAtomicCounterBuffers = */ 0,
        /* .MaxTessControlAtomicCounterBuffers = */ 0,
        /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
        /* .MaxGeometryAtomicCounterBuffers = */ 0,
        /* .MaxFragmentAtomicCounterBuffers = */ 1,
        /* .MaxCombinedAtomicCounterBuffers = */ 1,
        /* .MaxAtomicCounterBufferSize = */ 16384,
        /* .MaxTransformFeedbackBuffers = */ 4,
        /* .MaxTransformFeedbackInterleavedComponents = */ 64,
        /* .MaxCullDistances = */ 8,
        /* .MaxCombinedClipAndCullDistances = */ 8,
        /* .MaxSamples = */ 4,
        /* .MaxMeshOutputVerticesNV */ 1024,
        /* .MaxMeshOutputPrimitivesNV */ 1024,
        /* .MaxMeshWorkGroupSizeX_NV */ 1024,
        /* .MaxMeshWorkGroupSizeY_NV */ 1024,
        /* .MaxMeshWorkGroupSizeZ_NV */ 1024,
        /* .MaxTaskWorkGroupSizeX_NV */ 1024,
        /* .MaxTaskWorkGroupSizeY_NV */ 1024,
        /* .MaxTaskWorkGroupSizeZ_NV */ 1024,
        /* .MaxMeshViewCountNV */ 1024,
        /* .limits = */
        {
            /* .nonInductiveForLoops = */ true,
            /* .whileLoops = */ true,
            /* .doWhileLoops = */ true,
            /* .generalUniformIndexing = */ true,
            /* .generalAttributeMatrixVectorIndexing = */ true,
            /* .generalVaryingIndexing = */ true,
            /* .generalSamplerIndexing = */ true,
            /* .generalVariableIndexing = */ true,
            /* .generalConstantMatrixVectorIndexing = */ true,
        }};

    ShaderpackResourcesData load_dynamic_resources_file(const FolderAccessorBase* folder_access);

    ntl::Result<RendergraphData> load_rendergraph_file(const FolderAccessorBase* folder_access);

    rx::vector<PipelineCreateInfo> load_pipeline_files(const FolderAccessorBase* folder_access);
    rx::optional<PipelineCreateInfo> load_single_pipeline(const FolderAccessorBase* folder_access, const rx::string& pipeline_path);

    rx::vector<MaterialData> load_material_files(const FolderAccessorBase* folder_access);
    MaterialData load_single_material(const FolderAccessorBase* folder_access, const rx::string& material_path);

    void fill_in_render_target_formats(RenderpackData& data) {
        const auto& textures = data.resources.render_targets;

        data.graph_data.passes.each_fwd([&](RenderPassCreateInfo& pass) {
            pass.texture_outputs.each_fwd([&](TextureAttachmentInfo& output) {
                if(output.name == BACKBUFFER_NAME) {
                    // Backbuffer is a special snowflake
                    return true;

                } else if(output.name == SCENE_OUTPUT_RT_NAME) {
                    // Another special snowflake
                    return true;
                    // TODO: Figure out how to tell the loader about all the builtin resources
                }

                rx::optional<PixelFormatEnum> pixel_format;
                textures.each_fwd([&](const TextureCreateInfo& texture_info) {
                    if(texture_info.name == output.name) {
                        pixel_format = texture_info.format.pixel_format;
                        return false;
                    }

                    return true;
                });

                if(pixel_format) {
                    output.pixel_format = *pixel_format;
                } else {
                    NOVA_LOG(ERROR) << "Render pass " << pass.name.data() << " is trying to use texture " << output.name.data()
                                    << ", but it's not in the render graph's dynamic texture list";
                }

                return true;
            });

            if(pass.depth_texture) {
                rx::optional<PixelFormatEnum> pixel_format;
                textures.each_fwd([&](const TextureCreateInfo& texture_info) {
                    if(texture_info.name == pass.depth_texture->name) {
                        pixel_format = texture_info.format.pixel_format;
                        return false;
                    }

                    return true;
                });

                if(pixel_format) {
                    pass.depth_texture->pixel_format = *pixel_format;
                }
            }
        });
    }

    void cache_pipelines_by_renderpass(RenderpackData& data);

    RenderpackData load_shaderpack_data(const rx::string& shaderpack_name) {
        const FolderAccessorBase* folder_access = VirtualFilesystem::get_instance()->get_folder_accessor(shaderpack_name);

        // The shaderpack has a number of items: There's the shaders themselves, of course, but there's so, so much more
        // What else is there?
        // - resources.json, to describe the dynamic resources that a shaderpack needs
        // - passes.json, to describe the frame graph itself
        // - All the pipeline descriptions
        // - All the material descriptions
        //
        // All these things are loaded from the filesystem

        RenderpackData data{};
        data.resources = load_dynamic_resources_file(folder_access);
        const auto& graph_data = load_rendergraph_file(folder_access);
        if(graph_data) {
            data.graph_data = *graph_data;
        } else {
            NOVA_LOG(ERROR) << "Could not load render graph file. Error: " << graph_data.error.to_string();
        }
        data.pipelines = load_pipeline_files(folder_access);
        data.materials = load_material_files(folder_access);

        fill_in_render_target_formats(data);

        cache_pipelines_by_renderpass(data);

        return data;
    }

    rx::optional<ShaderpackResourcesData> load_dynamic_resources_file(FolderAccessorBase* folder_access) {
        NOVA_LOG(TRACE) << "load_dynamic_resource_file called";
        const rx::string resources_string = folder_access->read_text_file("resources.json");
        try {
            auto json_resources = nlohmann::json::parse(resources_string.data());
            const ValidationReport report = validate_shaderpack_resources_data(json_resources);
            print(report);
            if(!report.errors.empty()) {
                return rx::nullopt;
            }

            return rx::optional(json_resources.get<ShaderpackResourcesData>());
        }
        catch(nlohmann::json::parse_error& err) {
            NOVA_LOG(ERROR) << "Could not parse your renderpack's resources.json: " << err.what();
            return rx::nullopt;
        }
    }

    ntl::Result<RendergraphData> load_rendergraph_file(FolderAccessorBase* folder_access) {
        NOVA_LOG(TRACE) << "load_passes_file called";
        const auto passes_bytes = folder_access->read_text_file("rendergraph.json");
        try {
            const auto json_passes = nlohmann::json::parse(passes_bytes.data(), passes_bytes.data() + passes_bytes.size());

            auto rendergraph_file = json_passes.get<RendergraphData>();

            bool writes_to_scene_output_rt = false;
            rendergraph_file.passes.each_fwd([&](const RenderPassCreateInfo& pass) {
                // Check if this pass writes to the scene output RT
                pass.texture_outputs.each_fwd([&](const TextureAttachmentInfo& tex) {
                    if(tex.name == SCENE_OUTPUT_RT_NAME) {
                        writes_to_scene_output_rt = true;
                        return false;
                    }

                    return true;
                });

                if(writes_to_scene_output_rt) {
                    return false;

                } else {
                    return true;
                }
            });

            if(writes_to_scene_output_rt) {
                return ntl::Result<RendergraphData>(rendergraph_file);

            } else {
                return ntl::Result<RendergraphData>(
                    MAKE_ERROR("At least one pass must write to the render target named {:s}", SCENE_OUTPUT_RT_NAME));
            }
        }
        catch(nlohmann::json::parse_error& err) {
            return ntl::Result<RendergraphData>(MAKE_ERROR("Could not parse your shaderpack's passes.json: {:s}", err.what()));
        }
    }

    rx::vector<PipelineCreateInfo> load_pipeline_files(FolderAccessorBase* folder_access) {
        NOVA_LOG(TRACE) << "load_pipeline_files called";

        rx::vector<rx::string> potential_pipeline_files = folder_access->get_all_items_in_folder("materials");

        rx::vector<PipelineCreateInfo> output;

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        output.reserve(potential_pipeline_files.size());

        potential_pipeline_files.each_fwd([&](const rx::string& potential_file) {
            if(potential_file.ends_with(".pipeline")) {
                // Pipeline file!
                const auto& pipeline = load_single_pipeline(folder_access, potential_file);
                if(pipeline) {
                    output.push_back(*pipeline);
                }
            }
        });

        return output;
    }

    rx::optional<PipelineCreateInfo> load_single_pipeline(FolderAccessorBase* folder_access, const rx::string& pipeline_path) {
        NOVA_LOG(TRACE) << "Task to load pipeline " << pipeline_path.data() << " started";
        const auto pipeline_bytes = folder_access->read_text_file(pipeline_path);

        auto json_pipeline = nlohmann::json::parse(pipeline_bytes.data(), pipeline_bytes.data() + pipeline_bytes.size());
        NOVA_LOG(TRACE) << "Parsed JSON from disk for pipeline " << pipeline_path.data();
        const ValidationReport report = validate_graphics_pipeline(json_pipeline);
        NOVA_LOG(TRACE) << "Finished validating JSON for pipeline " << pipeline_path.data();
        print(report);
        if(!report.errors.empty()) {
            NOVA_LOG(ERROR) << "Loading pipeline file " << pipeline_path.data() << " failed";
            for(const auto& err : report.errors) {
                NOVA_LOG(ERROR) << err;
            }
            return {};
        }

        auto new_pipeline = json_pipeline.get<PipelineCreateInfo>();
        NOVA_LOG(TRACE) << "Parsed JSON into pipeline_data for pipeline " << pipeline_path.data();
        new_pipeline.vertex_shader.source = load_shader_file(new_pipeline.vertex_shader.filename,
                                                             folder_access,
                                                             rhi::ShaderStage::Vertex,
                                                             new_pipeline.defines);

        if(new_pipeline.geometry_shader) {
            (*new_pipeline.geometry_shader).source = load_shader_file((*new_pipeline.geometry_shader).filename,
                                                                      folder_access,
                                                                      rhi::ShaderStage::Geometry,
                                                                      new_pipeline.defines);
        }

        if(new_pipeline.tessellation_control_shader) {
            (*new_pipeline.tessellation_control_shader).source = load_shader_file((*new_pipeline.tessellation_control_shader).filename,
                                                                                  folder_access,
                                                                                  rhi::ShaderStage::TessellationControl,
                                                                                  new_pipeline.defines);
        }
        if(new_pipeline.tessellation_evaluation_shader) {
            (*new_pipeline.tessellation_evaluation_shader)
                .source = load_shader_file((*new_pipeline.tessellation_evaluation_shader).filename,
                                           folder_access,
                                           rhi::ShaderStage::TessellationEvaluation,
                                           new_pipeline.defines);
        }

        if(new_pipeline.fragment_shader) {
            (*new_pipeline.fragment_shader).source = load_shader_file((*new_pipeline.fragment_shader).filename,
                                                                      folder_access,
                                                                      rhi::ShaderStage::Fragment,
                                                                      new_pipeline.defines);
        }

        NOVA_LOG(TRACE) << "Load of pipeline " << pipeline_path.data() << " succeeded";

        return new_pipeline;
    }

    EShLanguage to_glslang_shader_stage(rhi::ShaderStage stage);

    rx::vector<uint32_t> load_shader_file(const rx::string& filename,
                                          FolderAccessorBase* folder_access,
                                          const rhi::ShaderStage stage,
                                          const rx::vector<rx::string>& defines) {
        // Be sure that we have glslang when we need to compile shaders
        glslang::InitializeProcess();

        const auto glslang_stage = to_glslang_shader_stage(stage);
        glslang::TShader shader(glslang_stage);

        rx::string shader_source = folder_access->read_text_file(filename);

        auto* shader_source_data = shader_source.data();
        shader.setStrings(&shader_source_data, 1);

        // Check the extension to know what kind of shader file the user has provided. SPIR-V files can be loaded
        // as-is, but GLSL, GLSL ES, and HLSL files need to be transpiled to SPIR-V
        if(filename.ends_with(".spirv")) {
            // SPIR-V file!

            rx::vector<uint8_t> bytes = folder_access->read_file(filename);
            auto view = bytes.disown();
            return rx::vector<uint32_t>(view);
        }
        if(filename.ends_with(".hlsl")) {
            shader.setEnvInput(glslang::EShSourceHlsl, glslang_stage, glslang::EShClientVulkan, 100);
            shader.setHlslIoMapping(true);

        } else {
            // GLSL files have a lot of possible extensions, but SPIR-V and HLSL don't!
            shader.setEnvInput(glslang::EShSourceGlsl, glslang_stage, glslang::EShClientVulkan, 100);
        }

        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);

        // TODO: Query the runtime for what version of SPIR-V we should target
        // For now just target the one that Works On My Machine
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);

        shader.setEntryPoint("main");

        const bool shader_compiled = shader.parse(&DEFAULT_BUILT_IN_RESOURCE,
                                                  450,
                                                  ECoreProfile,
                                                  false,
                                                  false,
                                                  EShMessages(EShMsgVulkanRules | EShMsgSpvRules));

        const char* info_log = shader.getInfoLog();
        if(std::strlen(info_log) > 0) {
            const char* info_debug_log = shader.getInfoDebugLog();
            NOVA_LOG(INFO) << filename.data() << " compilation messages:\n" << info_log << "\n" << info_debug_log;
        }

        if(!shader_compiled) {
            NOVA_LOG(ERROR) << "Could not load shader " << filename.data() << ": Shader compilation failed";
            return {};
        }

        glslang::TProgram program;
        program.addShader(&shader);
        const bool shader_linked = program.link(EShMsgDefault);
        if(!shader_linked) {
            const char* program_info_log = program.getInfoLog();
            const char* program_debug_info_log = program.getInfoDebugLog();
            NOVA_LOG(ERROR) << "Program failed to link: " << program_info_log << "\n" << program_debug_info_log;
        }

        // Using std::vector is okay here because we have to interface with `glslang`
        std::vector<uint32_t> spirv_std;
        GlslangToSpv(*program.getIntermediate(glslang_stage), spirv_std);

        const auto num_spirv_bytes = spirv_std.size() * sizeof(uint32_t);
        rx_byte* spirv_bytes = rx::memory::g_system_allocator->allocate(num_spirv_bytes);
        memcpy(spirv_bytes, spirv_std.data(), num_spirv_bytes);

        const rx::memory::view spirv_view{&rx::memory::g_system_allocator, spirv_bytes, spirv_std.size() * sizeof(uint32_t)};

        return rx::vector<uint32_t>(spirv_view);
    }

    rx::vector<MaterialData> load_material_files(const std::shared_ptr<FolderAccessorBase>& folder_access) {
        rx::vector<rx::string> potential_material_files = folder_access->get_all_items_in_folder("materials");

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        rx::vector<MaterialData> output;
        output.reserve(potential_material_files.size());

        for(const rx::string& potential_file : potential_material_files) {
            if(potential_file.extension() == ".mat") {
                const MaterialData& material = load_single_material(folder_access, potential_file);
                output.push_back(material);
            }
        }

        return output;
    }

    MaterialData load_single_material(const std::shared_ptr<FolderAccessorBase>& folder_access, const rx::string& material_path) {
        const rx::string material_text = folder_access->read_text_file(material_path);

        auto json_material = nlohmann::json::parse(material_text);
        const auto report = validate_material(json_material);
        print(report);
        if(!report.errors.empty()) {
            // There were errors, this material can't be loaded
            loading_failed = true;
            NOVA_LOG(TRACE) << "Load of material " << material_path << " failed";
            return {};
        }

        auto material = json_material.get<MaterialData>();
        material.name = material_path.stem().string();
        NOVA_LOG(TRACE) << "Load of material " << material_path << " succeeded";
        return material;
    }

    void cache_pipelines_by_renderpass(RenderpackData& data) {
        for(const auto& pipeline_info : data.pipelines) {
            for(auto& renderpass_info : data.graph_data.passes) {
                if(pipeline_info.pass == renderpass_info.name) {
                    renderpass_info.pipeline_names.emplace_back(pipeline_info.pass);
                }
            }
        }
    }

    EShLanguage to_glslang_shader_stage(const rhi::ShaderStage stage) {
        switch(stage) {
            case rhi::ShaderStage::Vertex:
                return EShLangVertex;

            case rhi::ShaderStage::TessellationControl:
                return EShLangTessControl;
            case rhi::ShaderStage::TessellationEvaluation:
                return EShLangTessEvaluation;

            case rhi::ShaderStage::Geometry:
                return EShLangGeometry;

            case rhi::ShaderStage::Fragment:
                return EShLangFragment;

            case rhi::ShaderStage::Compute:
                return EShLangCompute;

            case rhi::ShaderStage::Raygen:
                return EShLangRayGenNV;

            case rhi::ShaderStage::AnyHit:
                return EShLangAnyHitNV;

            case rhi::ShaderStage::ClosestHit:
                return EShLangClosestHitNV;

            case rhi::ShaderStage::Miss:
                return EShLangMissNV;

            case rhi::ShaderStage::Intersection:
                return EShLangIntersectNV;

            case rhi::ShaderStage::Task:
                return EShLangTaskNV;

            case rhi::ShaderStage::Mesh:
                return EShLangMeshNV;

            default:
                return EShLangCount;
        }
    }
} // namespace nova::renderer::shaderpack
