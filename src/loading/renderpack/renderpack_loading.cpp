#include "nova_renderer/loading/renderpack_loading.hpp"

#include <rx/core/json.h>
#include <rx/core/log.h>

// TODO: Replace this with our own COM macros
// #include <comdef.h> // TODO(janrupf): Was this really required?
#include <dxc/dxcapi.h>

#include "nova_renderer/constants.hpp"
#include "nova_renderer/filesystem/filesystem_helpers.hpp"
#include "nova_renderer/filesystem/folder_accessor.hpp"
#include "nova_renderer/filesystem/virtual_filesystem.hpp"

#include "../json_utils.hpp"
#include "minitrace.h"
#include "render_graph_builder.hpp"
#include "renderpack_validator.hpp"

namespace nova::renderer::renderpack {
    RX_LOG("RenderpackLoading", logger);

    using namespace filesystem;

    rx::optional<RenderpackResourcesData> load_dynamic_resources_file(FolderAccessorBase* folder_access);

    ntl::Result<RendergraphData> load_rendergraph_file(FolderAccessorBase* folder_access);

    rx::vector<PipelineData> load_pipeline_files(FolderAccessorBase* folder_access);
    rx::optional<PipelineData> load_single_pipeline(FolderAccessorBase* folder_access, const rx::string& pipeline_path);

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

                rx::optional<rhi::PixelFormat> pixel_format;
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
                rx::optional<rhi::PixelFormat> pixel_format;
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

    RenderpackData load_renderpack_data(const rx::string& renderpack_name) {
        MTR_SCOPE("load_renderpack_data", renderpack_name.data());

        FolderAccessorBase* folder_access = VirtualFilesystem::get_instance()->get_folder_accessor(renderpack_name);

        // The renderpack has a number of items: There's the shaders themselves, of course, but there's so, so much more
        // What else is there?
        // - resources.json, to describe the dynamic resources that a renderpack needs
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

    rx::optional<RenderpackResourcesData> load_dynamic_resources_file(FolderAccessorBase* folder_access) {
        MTR_SCOPE("load_dynamic_resource_file", "Self");

        const rx::string resources_string = folder_access->read_text_file(RESOURCES_FILE);

        auto json_resources = rx::json(resources_string);
        const ValidationReport report = validate_renderpack_resources_data(json_resources);
        print(report);
        if(!report.errors.is_empty()) {
            return rx::nullopt;
        }

        return RenderpackResourcesData::from_json(json_resources);
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

    rx::vector<PipelineData> load_pipeline_files(FolderAccessorBase* folder_access) {
        MTR_SCOPE("load_pipeline_files", "Self");

        rx::vector<rx::string> potential_pipeline_files = folder_access->get_all_items_in_folder("materials");

        rx::vector<PipelineData> output;

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

    rx::optional<PipelineData> load_single_pipeline(FolderAccessorBase* folder_access, const rx::string& pipeline_path) {
        MTR_SCOPE("load_single_pipeline", pipeline_path.data());

        const auto pipeline_bytes = folder_access->read_text_file(pipeline_path);

        auto json_pipeline = rx::json{pipeline_bytes};
        const ValidationReport report = validate_graphics_pipeline(json_pipeline);
        print(report);
        if(!report.errors.is_empty()) {
            logger(rx::log::level::k_error, "Loading pipeline file %s failed", pipeline_path);
            return rx::nullopt;
        }

        auto new_pipeline = json_pipeline.decode<PipelineData>({});
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
        data.pipelines.each_fwd([&](const PipelineData& pipeline_info) {
            data.graph_data.passes.each_fwd([&](RenderPassCreateInfo& renderpass_info) {
                if(pipeline_info.pass == renderpass_info.name) {
                    renderpass_info.pipeline_names.emplace_back(pipeline_info.pass);
                }
            });
        });
    }

    LPCWSTR to_hlsl_profile(const rhi::ShaderStage stage) {
        switch(stage) {
            case rhi::ShaderStage::Vertex:
                return L"vs_6_4";

            case rhi::ShaderStage::TessellationControl:
                return L"hs_6_4";

            case rhi::ShaderStage::TessellationEvaluation:
                return L"ds_6_4";

            case rhi::ShaderStage::Geometry:
                return L"gs_6_4";

            case rhi::ShaderStage::Fragment:
                return L"ps_6_4";

            case rhi::ShaderStage::Compute:
                return L"cs_6_4";

            case rhi::ShaderStage::Task:
                return L"as_6_4";

            case rhi::ShaderStage::Mesh:
                return L"ms_6_4";

            case rhi::ShaderStage::Raygen:
                [[fallthrough]];
            case rhi::ShaderStage::AnyHit:
                [[fallthrough]];
            case rhi::ShaderStage::ClosestHit:
                [[fallthrough]];
            case rhi::ShaderStage::Miss:
                [[fallthrough]];
            case rhi::ShaderStage::Intersection:
                [[fallthrough]];
            default:;
                logger(rx::log::level::k_error, "Unsupported shader stage %u", stage);
                return {};
        }
    }

    rx::vector<uint32_t> compile_shader(const rx::string& source, const rhi::ShaderStage stage, const rhi::ShaderLanguage source_language) {
        /*
         * Compile HLSL -> SPIR-V, using delicious DXC
         *
         * We use the old interface IDxcCompiler instead of IDxcCompiler3 because IDxcCompiler3 does not work, at all. It tells me that it
         * has a result, then it won't give me that result. I asked on the DirectX server and many other places, but apparently not even
         * Microsoft knows how to the new API for their compiler. Thus, I'm using the old and deprecated API - because it actually works
         */

        IDxcLibrary* lib;
        auto hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&lib));
        if(FAILED(hr)) {
            logger(rx::log::level::k_error, "Could not create DXC Library instance");
            return {};
        }

        IDxcCompiler* compiler;
        hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
        if(FAILED(hr)) {
            logger(rx::log::level::k_error, "Could not create DXC instance");
            return {};
        }

        IDxcBlobEncoding* encoding;
        hr = lib->CreateBlobWithEncodingFromPinned(source.data(), static_cast<UINT32>(source.size()), CP_UTF8, &encoding);
        if(FAILED(hr)) {
            logger(rx::log::level::k_error, "Could not create blob from shader");
            return {};
        }

        const auto profile = to_hlsl_profile(stage);

        rx::vector<LPCWSTR> args = rx::array{L"-spirv", L"-fspv-target-env=vulkan1.1", L"-fspv-reflect"};

        IDxcOperationResult* compile_result;
        hr = compiler->Compile(encoding,
                               L"unknown", // File name, for error messages
                               L"main",    // Entry point
                               profile,
                               args.data(),
                               static_cast<UINT32>(args.size()),
                               nullptr,
                               0,
                               nullptr,
                               &compile_result);
        if(FAILED(hr)) {
            logger(rx::log::level::k_error, "Could not compile shader");
            return {};
        }

        compile_result->GetStatus(&hr);
        if(SUCCEEDED(hr)) {
            IDxcBlob* result_blob;
            hr = compile_result->GetResult(&result_blob);
            rx::vector<uint32_t> spirv{result_blob->GetBufferSize() / sizeof(uint32_t)};
            memcpy(spirv.data(), result_blob->GetBufferPointer(), result_blob->GetBufferSize());
            return spirv;

        } else {
            IDxcBlobEncoding* error_buffer;
            compile_result->GetErrorBuffer(&error_buffer);
            logger(rx::log::level::k_error, "Error compiling shader:\n%s\n", static_cast<char const*>(error_buffer->GetBufferPointer()));
            error_buffer->Release();

            return {};
        }
    }
} // namespace nova::renderer::renderpack
