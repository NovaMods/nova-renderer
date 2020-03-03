#include "nova_renderer/loading/renderpack_loading.hpp"

#include <rx/core/json.h>
#include <rx/core/log.h>

// yolo
#include <wrl/client.h>

#include "nova_renderer/constants.hpp"
#include "nova_renderer/filesystem/filesystem_helpers.hpp"
#include "nova_renderer/filesystem/folder_accessor.hpp"
#include "nova_renderer/filesystem/virtual_filesystem.hpp"

#include "../external/dxc/include/dxc/dxcapi.h"
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

    rx::vector<uint32_t> compile_shader(const rx::string& source, const rhi::ShaderStage stage, const rhi::ShaderLanguage source_language) {
        MTR_SCOPE("compile_shader", "Self");

        // TODO: Our own equivalent for ComPtr

        Microsoft::WRL::ComPtr<IDxcUtils> dxc_utils;
        auto hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxc_utils));
        if(FAILED(hr)) {
            logger(rx::log::level::k_error, "Could not create DXC Utils instance");
            return {};
        }

        Microsoft::WRL::ComPtr<IDxcCompiler3> dxc;
        hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc));
        if(FAILED(hr)) {
            logger(rx::log::level::k_error, "Could not create DXC instance");
            return {};
        }

        Microsoft::WRL::ComPtr<IDxcBlobEncoding> shader_blob;
        hr = dxc_utils->CreateBlob(source.data(), source.size() * sizeof(char), 0, &shader_blob);
        if(FAILED(hr)) {
            logger(rx::log::level::k_error, "Could not create blob from shader");
            return {};
        }

        DxcBuffer buffer{};
        buffer.Ptr = shader_blob->GetBufferPointer();
        buffer.Size = shader_blob->GetBufferSize();
        BOOL data;
        shader_blob->GetEncoding(&data, &buffer.Encoding);

        LPCWSTR args[3] = {L"spirv",
                           L"fspv-reflect"
                           L"Tvertex"};

        Microsoft::WRL::ComPtr<IDxcResult> result;
        hr = dxc->Compile(&buffer, args, 3, nullptr, IID_PPV_ARGS(&result));
        if(FAILED(hr)) {
            logger(rx::log::level::k_error, "Could not compile shader");
            Microsoft::WRL::ComPtr<IDxcBlobEncoding> error_blob;
            hr = result->GetErrorBuffer(&error_blob);
            if(FAILED(hr) || !error_blob) {
                logger(rx::log::level::k_error, "Couldn't even get the error message lmfao");
            } else {
                logger(rx::log::level::k_error, "Compilation failed with error: %s", error_blob->GetBufferPointer());
            }

            return {};
        }

        Microsoft::WRL::ComPtr<IDxcBlob> code;
        result->GetResult(&code);

        rx::vector<uint32_t> spirv_rx{code->GetBufferSize()};
        memcpy(spirv_rx.data(), code->GetBufferPointer(), code->GetBufferSize());
        return spirv_rx;
    }
} // namespace nova::renderer::renderpack
