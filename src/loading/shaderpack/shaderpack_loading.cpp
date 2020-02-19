#include "nova_renderer/loading/shaderpack_loading.hpp"

#define ENABLE_HLSL
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <rx/core/json.h>
#include <rx/core/log.h>

#include "nova_renderer/constants.hpp"
#include "nova_renderer/filesystem/filesystem_helpers.hpp"
#include "nova_renderer/filesystem/folder_accessor.hpp"
#include "nova_renderer/filesystem/virtual_filesystem.hpp"

#include "../json_utils.hpp"
#include "minitrace.h"
#include "render_graph_builder.hpp"
#include "shaderpack_validator.hpp"

namespace nova::renderer::shaderpack {
    RX_LOG("ShaderpackLoading", logger);

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

    rx::optional<ShaderpackResourcesData> load_dynamic_resources_file(FolderAccessorBase* folder_access);

    ntl::Result<RendergraphData> load_rendergraph_file(FolderAccessorBase* folder_access);

    rx::vector<PipelineCreateInfo> load_pipeline_files(FolderAccessorBase* folder_access);
    rx::optional<PipelineCreateInfo> load_single_pipeline(FolderAccessorBase* folder_access, const rx::string& pipeline_path);

    rx::vector<MaterialData> load_material_files(FolderAccessorBase* folder_access);
    MaterialData load_single_material(FolderAccessorBase* folder_access, const rx::string& material_path);

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
                    logger(rx::log::level::k_error,
                           "Render pass %s is trying to use texture %s, but it's not in the render graph's dynamic texture list",
                           pass.name,
                           output.name);
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
        MTR_SCOPE("load_shaderpack_data", shaderpack_name.data());

        FolderAccessorBase* folder_access = VirtualFilesystem::get_instance()->get_folder_accessor(shaderpack_name);

        // The shaderpack has a number of items: There's the shaders themselves, of course, but there's so, so much more
        // What else is there?
        // - resources.json, to describe the dynamic resources that a shaderpack needs
        // - passes.json, to describe the frame graph itself
        // - All the pipeline descriptions
        // - All the material descriptions
        //
        // All these things are loaded from the filesystem

        RenderpackData data{};
        data.resources = *load_dynamic_resources_file(folder_access);
        const auto& graph_data = load_rendergraph_file(folder_access);
        if(graph_data) {
            data.graph_data = *graph_data;
        } else {
            logger(rx::log::level::k_error, "Could not load render graph file. Error: %s", graph_data.error.to_string());
        }
        data.pipelines = load_pipeline_files(folder_access);
        data.materials = load_material_files(folder_access);

        fill_in_render_target_formats(data);

        cache_pipelines_by_renderpass(data);

        return data;
    }

    rx::optional<ShaderpackResourcesData> load_dynamic_resources_file(FolderAccessorBase* folder_access) {
        MTR_SCOPE("load_dynamic_resource_file", "Self");

        const rx::string resources_string = folder_access->read_text_file(RESOURCES_FILE);

        auto json_resources = rx::json(resources_string);
        const ValidationReport report = validate_shaderpack_resources_data(json_resources);
        print(report);
        if(!report.errors.is_empty()) {
            return rx::nullopt;
        }

        return ShaderpackResourcesData::from_json(json_resources);
    }

    ntl::Result<RendergraphData> load_rendergraph_file(FolderAccessorBase* folder_access) {
        MTR_SCOPE("load_rendergraph_file", "Self");

        const auto passes_bytes = folder_access->read_text_file("rendergraph.json");

        const auto json_passes = rx::json(passes_bytes);

        auto rendergraph_file = json_passes.decode<RendergraphData>({});

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
                MAKE_ERROR("At least one pass must write to the render target named %s", SCENE_OUTPUT_RT_NAME));
        }
    }

    rx::vector<PipelineCreateInfo> load_pipeline_files(FolderAccessorBase* folder_access) {
        MTR_SCOPE("load_pipeline_files", "Self");

        rx::vector<rx::string> potential_pipeline_files = folder_access->get_all_items_in_folder("materials");

        rx::vector<PipelineCreateInfo> output;

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        output.reserve(potential_pipeline_files.size());

        potential_pipeline_files.each_fwd([&](const rx::string& potential_file) {
            if(potential_file.ends_with(".pipeline")) {
                // Pipeline file!
                const auto pipeline_relative_path = rx::string::format("%s/%s", "materials", potential_file);
                const auto& pipeline = load_single_pipeline(folder_access, pipeline_relative_path);
                if(pipeline) {
                    output.push_back(*pipeline);
                }
            }
        });

        return output;
    }

    rx::optional<PipelineCreateInfo> load_single_pipeline(FolderAccessorBase* folder_access, const rx::string& pipeline_path) {
        MTR_SCOPE("load_single_pipeline", pipeline_path.data());

        const auto pipeline_bytes = folder_access->read_text_file(pipeline_path);

        auto json_pipeline = rx::json{pipeline_bytes};
        const ValidationReport report = validate_graphics_pipeline(json_pipeline);
        print(report);
        if(!report.errors.is_empty()) {
            logger(rx::log::level::k_error, "Loading pipeline file %s failed", pipeline_path);
            return rx::nullopt;
        }

        auto new_pipeline = json_pipeline.decode<PipelineCreateInfo>({});
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

        logger(rx::log::level::k_verbose, "Load of pipeline %s succeeded", pipeline_path);

        return new_pipeline;
    }

    EShLanguage to_glslang_shader_stage(rhi::ShaderStage stage);

    rx::vector<uint32_t> load_shader_file(const rx::string& filename,
                                          FolderAccessorBase* folder_access,
                                          const rhi::ShaderStage stage,
                                          const rx::vector<rx::string>& defines) {
        MTR_SCOPE("load_shader_file", filename.data());

        if(filename.ends_with(".spirv")) {
            // SPIR-V file!

            rx::vector<uint8_t> bytes = folder_access->read_file(filename);
            const auto view = bytes.disown();
            return rx::vector<uint32_t>{view};
        }

        rx::string shader_source = folder_access->read_text_file(filename);

        const auto& compiled_shader = [&] {
            if(filename.ends_with(".hlsl")) {
                return compile_shader(shader_source, stage, rhi::ShaderLanguage::Hlsl);

            } else {
                return compile_shader(shader_source, stage, rhi::ShaderLanguage::Glsl);
            }
        }();

        if(compiled_shader.is_empty()) {
            logger(rx::log::level::k_error, "Could not compile shader file %s", filename);
        }

        return compiled_shader;
    }

    rx::vector<MaterialData> load_material_files(FolderAccessorBase* folder_access) {
        MTR_SCOPE("load_material_files", "Self");

        rx::vector<rx::string> potential_material_files = folder_access->get_all_items_in_folder("materials");

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        rx::vector<MaterialData> output;
        output.reserve(potential_material_files.size());

        potential_material_files.each_fwd([&](const rx::string& potential_file) {
            if(potential_file.ends_with(".mat")) {
                const auto material_filename = rx::string::format("%s/%s", MATERIALS_DIRECTORY, potential_file);
                const MaterialData& material = load_single_material(folder_access, material_filename);
                output.push_back(material);
            }
        });

        return output;
    }

    MaterialData load_single_material(FolderAccessorBase* folder_access, const rx::string& material_path) {
        MTR_SCOPE("load_single_material", material_path.data());

        const rx::string material_text = folder_access->read_text_file(material_path);

        const auto json_material = rx::json{material_text};
        const auto report = validate_material(json_material);
        print(report);
        if(!report.errors.is_empty()) {
            // There were errors, this material can't be loaded
            logger(rx::log::level::k_error, "Load of material %s failed", material_path);
            return {};
        }

        const auto material_file_name = get_file_name(material_path);
        const auto material_extension_begin_idx = material_file_name.size() - 4; // ".mat"

        auto material = json_material.decode<MaterialData>({});
        material.name = material_file_name.substring(0, material_extension_begin_idx);

        material.passes.each_fwd([&](MaterialPass& pass) { pass.material_name = material.name; });

        logger(rx::log::level::k_verbose, "Load of material &s succeeded - name %s", material_path, material.name);
        return material;
    }

    void cache_pipelines_by_renderpass(RenderpackData& data) {
        data.pipelines.each_fwd([&](const PipelineCreateInfo& pipeline_info) {
            data.graph_data.passes.each_fwd([&](RenderPassCreateInfo& renderpass_info) {
                if(pipeline_info.pass == renderpass_info.name) {
                    renderpass_info.pipeline_names.emplace_back(pipeline_info.pass);
                }
            });
        });
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

    rx::vector<uint32_t> compile_shader(const rx::string& source, const rhi::ShaderStage stage, const glslang::EShSource source_language) {
        MTR_SCOPE("compile_shader", "Self");

        const auto glslang_stage = to_glslang_shader_stage(stage);

        // Be sure that we have glslang when we need to compile shaders
        glslang::InitializeProcess();

        glslang::TShader shader{glslang_stage};

        const auto* source_ptr = source.data();
        shader.setStrings(&source_ptr, 1);

        if(source_language == glslang::EShSourceHlsl) {
            shader.setEnvInput(glslang::EShSourceHlsl, glslang_stage, glslang::EShClientVulkan, 100);
            shader.setHlslIoMapping(true);

        } else if(source_language == glslang::EShSourceGlsl) {
            // GLSL files have a lot of possible extensions, but SPIR-V and HLSL don't!
            shader.setEnvInput(glslang::EShSourceGlsl, glslang_stage, glslang::EShClientVulkan, 100);

        } else {
            logger(rx::log::level::k_error, "Incompatible shader source language");
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
            logger(rx::log::level::k_info, "Shader compilation messages:\n%s\n%s", info_log, info_debug_log);
        }

        if(!shader_compiled) {
            logger(rx::log::level::k_error, "Could not compile shader");
            return {};
        }

        glslang::TProgram program;
        program.addShader(&shader);
        const bool shader_linked = program.link(EShMsgDefault);
        if(!shader_linked) {
            const char* program_info_log = program.getInfoLog();
            const char* program_debug_info_log = program.getInfoDebugLog();
            logger(rx::log::level::k_error, "Program failed to link: %s\n%s", program_info_log, program_debug_info_log);
        }

        // Using std::vector is okay here because we have to interface with `glslang`
        std::vector<uint32_t> spirv_std;
        GlslangToSpv(*program.getIntermediate(glslang_stage), spirv_std);

        rx::vector<uint32_t> spirv_rx(spirv_std.size());
        memcpy(spirv_rx.data(), spirv_std.data(), spirv_std.size() * sizeof(uint32_t));
        return spirv_rx;
    }
} // namespace nova::renderer::shaderpack
