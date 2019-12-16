/*!
 * \author ddubois
 * \date 21-Aug-18.
 */

#include "shaderpack_loading.hpp"

#include <glslang/Include/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include "../../tasks/task_scheduler.hpp"
#include "../folder_accessor.hpp"
#include "../json_utils.hpp"
#include "../loading_utils.hpp"
#include "../regular_folder_accessor.hpp"
#include "../zip_folder_accessor.hpp"
#include "SPIRV/GlslangToSpv.h"
#include "json_interop.hpp"
#include "render_graph_builder.hpp"
#include "shaderpack_validator.hpp"
#include "nova_renderer/constants.hpp"

namespace nova::renderer::shaderpack {
    // Removed from the GLSLang version we're using
    // TODO: Copy and fill in with values from the RHI so we don't accidentally limit a shader
    const TBuiltInResource default_built_in_resource = {
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

    std::shared_ptr<FolderAccessorBase> get_shaderpack_accessor(const fs::path& shaderpack_name);

    ShaderpackResourcesData load_dynamic_resources_file(const std::shared_ptr<FolderAccessorBase>& folder_access);

    ntl::Result<RendergraphData> load_rendergraph_file(const std::shared_ptr<FolderAccessorBase>& folder_access);

    std::vector<PipelineCreateInfo> load_pipeline_files(const std::shared_ptr<FolderAccessorBase>& folder_access);
    PipelineCreateInfo load_single_pipeline(const std::shared_ptr<FolderAccessorBase>& folder_access, const fs::path& pipeline_path);

    std::vector<MaterialData> load_material_files(const std::shared_ptr<FolderAccessorBase>& folder_access);
    MaterialData load_single_material(const std::shared_ptr<FolderAccessorBase>& folder_access, const fs::path& material_path);

    std::vector<uint32_t> load_shader_file(const fs::path& filename,
                                           const std::shared_ptr<FolderAccessorBase>& folder_access,
                                           EShLanguage stage,
                                           const std::vector<std::string>& defines);

    bool loading_failed = false;

    void fill_in_render_target_formats(ShaderpackData& data) {
        const auto& textures = data.resources.textures;

        for(auto& pass : data.graph_data.passes) {
            for(auto& output : pass.texture_outputs) {
                if(output.name == BACKBUFFER_NAME) {
                    // Backbuffer is a special snowflake
                    continue;

                } else if(output.name == "NovaFinal") {
                    // Another special snowflake
                    continue;
                    // TODO: Figure out how to tell the loader about all the builtin resources
                }

                if(const auto& tex_itr = std::find_if(textures.begin(),
                                                      textures.end(),
                                                      [&](const TextureCreateInfo& texture_info) {
                                                          return texture_info.name == output.name;
                                                      });
                   tex_itr != textures.end()) {
                    output.pixel_format = tex_itr->format.pixel_format;
                } else {
                    NOVA_LOG(ERROR) << "Render pass " << pass.name << " is trying to use texture " << output.name
                                    << ", but it's not in the render graph's dynamic texture list";
                }
            }

            if(pass.depth_texture) {
                if(const auto& tex_itr = std::find_if(textures.begin(),
                                                      textures.end(),
                                                      [&](const TextureCreateInfo& texture_info) {
                                                          return texture_info.name == pass.depth_texture->name;
                                                      });
                   tex_itr != textures.end()) {
                    pass.depth_texture->pixel_format = tex_itr->format.pixel_format;
                }
            }
        }
    }

    ShaderpackData load_shaderpack_data(const fs::path& shaderpack_name) {
        loading_failed = false;
        const std::shared_ptr<FolderAccessorBase> folder_access = get_shaderpack_accessor(shaderpack_name);

        // The shaderpack has a number of items: There's the shaders themselves, of course, but there's so, so much more
        // What else is there?
        // - resources.json, to describe the dynamic resources that a shaderpack needs
        // - passes.json, to describe the frame graph itself
        // - All the pipeline descriptions
        // - All the material descriptions
        //
        // All these things are loaded from the filesystem

        ShaderpackData data{};
        data.resources = load_dynamic_resources_file(folder_access);
        data.graph_data = load_rendergraph_file(folder_access).value; // TODO: Gracefully handle errors
        data.pipelines = load_pipeline_files(folder_access);
        data.materials = load_material_files(folder_access);

        fill_in_render_target_formats(data);

        return data;
    }

    std::shared_ptr<FolderAccessorBase> get_shaderpack_accessor(const fs::path& shaderpack_name) {
        fs::path path_to_shaderpack = shaderpack_name;

        // Where is the shaderpack, and what kind of folder is it in?
        if(is_zip_folder(path_to_shaderpack)) {
            // zip folder in shaderpacks folder
            path_to_shaderpack.replace_extension(".zip");
            return std::make_shared<ZipFolderAccessor>(path_to_shaderpack);
        }
        if(fs::exists(path_to_shaderpack)) {
            // regular folder in shaderpacks folder
            return std::make_shared<RegularFolderAccessor>(path_to_shaderpack);
        }

        return {};
    }

    ShaderpackResourcesData load_dynamic_resources_file(const std::shared_ptr<FolderAccessorBase>& folder_access) {
        NOVA_LOG(TRACE) << "load_dynamic_resource_file called";
        const std::string resources_string = folder_access->read_text_file("resources.json");
        try {
            auto json_resources = nlohmann::json::parse(resources_string.c_str());
            const ValidationReport report = validate_shaderpack_resources_data(json_resources);
            print(report);
            if(!report.errors.empty()) {
                loading_failed = true;
                return {};
            }

            return json_resources.get<ShaderpackResourcesData>();
        }
        catch(nlohmann::json::parse_error& err) {
            NOVA_LOG(ERROR) << "Could not parse your shaderpack's resources.json: " << err.what();
            loading_failed = true;
        }

        return {};
    }

    ntl::Result<RendergraphData> load_rendergraph_file(const std::shared_ptr<FolderAccessorBase>& folder_access) {
        NOVA_LOG(TRACE) << "load_passes_file called";
        const auto passes_bytes = folder_access->read_text_file("rendergraph.json");
        try {
            const auto json_passes = nlohmann::json::parse(passes_bytes);

            auto rendergraph_file = json_passes.get<RendergraphData>();

            const auto final_itr = std::find_if(rendergraph_file.passes.begin(),
                                                rendergraph_file.passes.end(),
                                                [&](const RenderPassCreateInfo& pass) {
                                                    const auto output_itr = std::find_if(pass.texture_outputs.begin(),
                                                                                         pass.texture_outputs.end(),
                                                                                         [](const TextureAttachmentInfo& tex) {
                                                                                             return tex.name == "NovaFinal";
                                                                                         });

                                                    return output_itr != pass.texture_outputs.end();
                                                });

            if(final_itr != rendergraph_file.passes.end()) {
                return ntl::Result<RendergraphData>(rendergraph_file);

            } else {
                return ntl::Result<RendergraphData>(ntl::NovaError("At least one pass must write to resource NovaFinal"));
            }
        }
        catch(nlohmann::json::parse_error& err) {
            return ntl::Result<RendergraphData>(MAKE_ERROR("Could not parse your shaderpack's passes.json: {:s}", err.what()));
        }
    }

    std::vector<PipelineCreateInfo> load_pipeline_files(const std::shared_ptr<FolderAccessorBase>& folder_access) {
        NOVA_LOG(TRACE) << "load_pipeline_files called";

        std::vector<fs::path> potential_pipeline_files = folder_access->get_all_items_in_folder("materials");

        std::vector<PipelineCreateInfo> output;

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        output.reserve(potential_pipeline_files.size());

        for(const fs::path& potential_file : potential_pipeline_files) {
            if(potential_file.extension() == ".pipeline") {
                // Pipeline file!
                const PipelineCreateInfo& pipeline = load_single_pipeline(folder_access, potential_file);
                output.push_back(pipeline);
            }
        }

        return output;
    }

    PipelineCreateInfo load_single_pipeline(const std::shared_ptr<FolderAccessorBase>& folder_access, const fs::path& pipeline_path) {
        NOVA_LOG(TRACE) << "Task to load pipeline " << pipeline_path << " started";
        const auto pipeline_bytes = folder_access->read_text_file(pipeline_path);

        auto json_pipeline = nlohmann::json::parse(pipeline_bytes);
        NOVA_LOG(TRACE) << "Parsed JSON from disk for pipeline " << pipeline_path;
        const ValidationReport report = validate_graphics_pipeline(json_pipeline);
        NOVA_LOG(TRACE) << "Finished validating JSON for pipeline " << pipeline_path;
        print(report);
        if(!report.errors.empty()) {
            loading_failed = true;
            NOVA_LOG(TRACE) << "Loading pipeline file " << pipeline_path << " failed";
            return {};
        }

        auto new_pipeline = json_pipeline.get<PipelineCreateInfo>();
        NOVA_LOG(TRACE) << "Parsed JSON into pipeline_data for pipeline " << pipeline_path;
        new_pipeline.vertex_shader.source = load_shader_file(new_pipeline.vertex_shader.filename,
                                                             folder_access,
                                                             EShLangVertex,
                                                             new_pipeline.defines);

        if(new_pipeline.geometry_shader) {
            (*new_pipeline.geometry_shader).source = load_shader_file((*new_pipeline.geometry_shader).filename,
                                                                      folder_access,
                                                                      EShLangGeometry,
                                                                      new_pipeline.defines);
        }

        if(new_pipeline.tessellation_control_shader) {
            (*new_pipeline.tessellation_control_shader).source = load_shader_file((*new_pipeline.tessellation_control_shader).filename,
                                                                                  folder_access,
                                                                                  EShLangTessControl,
                                                                                  new_pipeline.defines);
        }
        if(new_pipeline.tessellation_evaluation_shader) {
            (*new_pipeline.tessellation_evaluation_shader)
                .source = load_shader_file((*new_pipeline.tessellation_evaluation_shader).filename,
                                           folder_access,
                                           EShLangTessEvaluation,
                                           new_pipeline.defines);
        }

        if(new_pipeline.fragment_shader) {
            (*new_pipeline.fragment_shader).source = load_shader_file((*new_pipeline.fragment_shader).filename,
                                                                      folder_access,
                                                                      EShLangFragment,
                                                                      new_pipeline.defines);
        }

        NOVA_LOG(TRACE) << "Load of pipeline " << pipeline_path << " succeeded";

        return new_pipeline;
    }

    std::vector<uint32_t> load_shader_file(const fs::path& filename,
                                           const std::shared_ptr<FolderAccessorBase>& folder_access,
                                           const EShLanguage stage,
                                           const std::vector<std::string>& defines) {
        static std::unordered_map<EShLanguage, std::vector<fs::path>> extensions_by_shader_stage = {{EShLangVertex,
                                                                                                     {
                                                                                                         ".vert.spirv",
                                                                                                         ".vsh.spirv",
                                                                                                         ".vertex.spirv",

                                                                                                         ".vert",
                                                                                                         ".vsh",

                                                                                                         ".vertex",

                                                                                                         ".vert.hlsl",
                                                                                                         ".vsh.hlsl",
                                                                                                         ".vertex.hlsl",
                                                                                                     }},
                                                                                                    {EShLangFragment,
                                                                                                     {
                                                                                                         ".frag.spirv",
                                                                                                         ".fsh.spirv",
                                                                                                         ".fragment.spirv",

                                                                                                         ".frag",
                                                                                                         ".fsh",

                                                                                                         ".fragment",

                                                                                                         ".frag.hlsl",
                                                                                                         ".fsh.hlsl",
                                                                                                         ".fragment.hlsl",
                                                                                                     }},
                                                                                                    {EShLangGeometry,
                                                                                                     {
                                                                                                         ".geom.spirv",
                                                                                                         ".geo.spirv",
                                                                                                         ".geometry.spirv",

                                                                                                         ".geom",
                                                                                                         ".geo",

                                                                                                         ".geometry",

                                                                                                         ".geom.hlsl",
                                                                                                         ".geo.hlsl",
                                                                                                         ".geometry.hlsl",
                                                                                                     }},
                                                                                                    {EShLangTessEvaluation,
                                                                                                     {
                                                                                                         ".tese.spirv",
                                                                                                         ".tse.spirv",
                                                                                                         ".tess_eval.spirv",

                                                                                                         ".tese",
                                                                                                         ".tse",

                                                                                                         ".tess_eval",

                                                                                                         ".tese.hlsl",
                                                                                                         ".tse.hlsl",
                                                                                                         ".tess_eval.hlsl",
                                                                                                     }},
                                                                                                    {EShLangTessControl,
                                                                                                     {
                                                                                                         ".tesc.spirv",
                                                                                                         ".tsc.spirv",
                                                                                                         ".tess_control.spirv",

                                                                                                         ".tesc",
                                                                                                         ".tsc",

                                                                                                         ".tess_control",

                                                                                                         ".tesc.hlsl",
                                                                                                         ".tsc.hlsl",
                                                                                                         ".tess_control.hlsl",
                                                                                                     }}};

        std::vector<fs::path> extensions_for_current_stage = extensions_by_shader_stage.at(stage);

        for(const fs::path& extension : extensions_for_current_stage) {
            fs::path full_filename = filename;
            full_filename.replace_extension(extension);

            if(!folder_access->does_resource_exist(full_filename)) {
                continue;
            }

            glslang::TShader shader(stage);

            // Check the extension to know what kind of shader file the user has provided. SPIR-V files can be loaded
            // as-is, but GLSL, GLSL ES, and HLSL files need to be transpiled to SPIR-V
            if(extension.string().find(".spirv") != std::string::npos) {
                // SPIR-V file!
                // TODO: figure out how to handle defines with SPIRV
                return folder_access->read_spirv_file(full_filename);
            }
            if(extension.string().find(".hlsl") != std::string::npos) {
                shader.setEnvInput(glslang::EShSourceHlsl, stage, glslang::EShClientVulkan, 0);
            } else {
                // GLSL files have a lot of possible extensions, but SPIR-V and HLSL don't!
                shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 0);
            }

            std::string shader_source = folder_access->read_text_file(full_filename);
            std::string::size_type version_pos = shader_source.find("#version");
            std::string::size_type inject_pos = 0;
            if(version_pos != std::string::npos) {
                std::string::size_type break_after_version_pos = shader_source.find('\n', version_pos);
                if(break_after_version_pos != std::string::npos) {
                    inject_pos = break_after_version_pos + 1;
                }
            }
            for(auto i = defines.crbegin(); i != defines.crend(); ++i) {
                shader_source.insert(inject_pos, "#define " + *i + "\n");
            }

            auto* shader_source_data = shader_source.data();
            shader.setStrings(&shader_source_data, 1);
            const bool shader_compiled = shader.parse(&default_built_in_resource,
                                                      450,
                                                      ECoreProfile,
                                                      false,
                                                      false,
                                                      EShMessages(EShMsgVulkanRules | EShMsgSpvRules));

            const char* info_log = shader.getInfoLog();
            if(std::strlen(info_log) > 0) {
                const char* info_debug_log = shader.getInfoDebugLog();
                NOVA_LOG(INFO) << full_filename.string() << " compilation messages:\n" << info_log << "\n" << info_debug_log;
            }

            if(!shader_compiled) {
                NOVA_LOG(ERROR) << "Shader compilation failed";
            }

            glslang::TProgram program;
            program.addShader(&shader);
            const bool shader_linked = program.link(EShMsgDefault);
            if(!shader_linked) {
                const char* program_info_log = program.getInfoLog();
                const char* program_debug_info_log = program.getInfoDebugLog();
                NOVA_LOG(ERROR) << "Program failed to link: " << program_info_log << "\n" << program_debug_info_log;
            }

            std::vector<uint32_t> spirv_std;
            GlslangToSpv(*program.getIntermediate(stage), spirv_std);

            std::vector<uint32_t> spirv(spirv_std.begin(), spirv_std.end());

            fs::path dump_filename = filename.filename();
            dump_filename.replace_extension(std::to_string(stage) + ".spirv.generated");
            write_to_file(spirv, dump_filename);

            return spirv;
        }

        NOVA_LOG(ERROR) << "Could not find shader " << filename.c_str();
        return {};
    }

    std::vector<MaterialData> load_material_files(const std::shared_ptr<FolderAccessorBase>& folder_access) {
        std::vector<fs::path> potential_material_files = folder_access->get_all_items_in_folder("materials");

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        std::vector<MaterialData> output;
        output.reserve(potential_material_files.size());

        for(const fs::path& potential_file : potential_material_files) {
            if(potential_file.extension() == ".mat") {
                const MaterialData& material = load_single_material(folder_access, potential_file);
                output.push_back(material);
            }
        }

        return output;
    }

    MaterialData load_single_material(const std::shared_ptr<FolderAccessorBase>& folder_access, const fs::path& material_path) {
        const std::string material_text = folder_access->read_text_file(material_path);

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
        material.name = material_path.stem().string().c_str();
        NOVA_LOG(TRACE) << "Load of material " << material_path << " succeeded";
        return material;
    }
} // namespace nova::renderer::shaderpack
