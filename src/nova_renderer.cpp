/*!
 * \author ddubois
 * \date 03-Sep-18.
 */

#include <array>
#include <future>

#include "nova_renderer/nova_renderer.hpp"

#include <glslang/MachineIndependent/Initialize.h>
#include <spirv_cross/spirv_glsl.hpp>
#include "loading/shaderpack/shaderpack_loading.hpp"

#if defined(NOVA_WINDOWS)
#include "render_engine/dx12/dx12_render_engine.hpp"
#endif

#include "debugging/renderdoc.hpp"
#include "render_engine/vulkan/vulkan_render_engine.hpp"

#include <minitrace/minitrace.h>
#include "loading/shaderpack/render_graph_builder.hpp"
#include "render_engine/gl3/gl3_render_engine.hpp"
#include "util/logger.hpp"

namespace nova::renderer {
    std::unique_ptr<NovaRenderer> NovaRenderer::instance;

    NovaRenderer::NovaRenderer(NovaSettings settings) : render_settings(settings) {

        mtr_init("trace.json");

        MTR_META_PROCESS_NAME("NovaRenderer");
        MTR_META_THREAD_NAME("Main");

        MTR_SCOPE("Init", "nova_renderer::nova_renderer");

        if(settings.debug.renderdoc.enabled) {
            MTR_SCOPE("Init", "LoadRenderdoc");
            auto rd_load_result = load_renderdoc(settings.debug.renderdoc.renderdoc_dll_path);

            rd_load_result
                .map([&](RENDERDOC_API_1_3_0* api) {
                    render_doc = api;

                    render_doc->SetCaptureFilePathTemplate(settings.debug.renderdoc.capture_path.c_str());

                    RENDERDOC_InputButton capture_key[] = {eRENDERDOC_Key_F12, eRENDERDOC_Key_PrtScrn};
                    render_doc->SetCaptureKeys(capture_key, 2);

                    render_doc->SetCaptureOptionU32(eRENDERDOC_Option_AllowFullscreen, 1U);
                    render_doc->SetCaptureOptionU32(eRENDERDOC_Option_AllowVSync, 1U);
                    render_doc->SetCaptureOptionU32(eRENDERDOC_Option_VerifyMapWrites, 1U);
                    render_doc->SetCaptureOptionU32(eRENDERDOC_Option_SaveAllInitials, 1U);
                    render_doc->SetCaptureOptionU32(eRENDERDOC_Option_APIValidation, 1U);

                    NOVA_LOG(INFO) << "Loaded RenderDoc successfully";

                    return 0;
                })
                .on_error([](const NovaError& error) { NOVA_LOG(ERROR) << error.to_string(); });
        }

        switch(settings.api) {
            case GraphicsApi::Dx12:
#if defined(NOVA_WINDOWS)
            {
                MTR_SCOPE("Init", "InitDirect3D12RenderEngine");
                rhi = std::make_unique<rhi::DX12RenderEngine>(render_settings);
            } break;
#else
                NOVA_LOG(WARN) << "You selected the DX12 graphics API, but your system doesn't support it. Defaulting to Vulkan";
                [[fallthrough]];
#endif
            case GraphicsApi::Vulkan: {
                MTR_SCOPE("Init", "InitVulkanRenderEngine");
                rhi = std::make_unique<rhi::VulkanRenderEngine>(render_settings);
            } break;

            case GraphicsApi::Gl2: {
                MTR_SCOPE("Init", "InitGL3RenderEngine");
                rhi = std::make_unique<rhi::Gl3RenderEngine>(render_settings);
            } break;
        }
    }

    NovaRenderer::~NovaRenderer() { mtr_shutdown(); }

    NovaSettings& NovaRenderer::get_settings() { return render_settings; }

    void NovaRenderer::execute_frame() const {
        mtr_flush();

        MTR_SCOPE("RenderLoop", "execute_frame");
    }

    void NovaRenderer::load_shaderpack(const std::string& shaderpack_name) {
        MTR_SCOPE("ShaderpackLoading", "load_shaderpack");
        glslang::InitializeProcess();

        const shaderpack::ShaderpackData data = shaderpack::load_shaderpack_data(fs::path(shaderpack_name));

        if(shaderpack_loaded) {
            destroy_render_passes();

            destroy_dynamic_resources();

            NOVA_LOG(DEBUG) << "Resources from old shaderpacks destroyed";
        }

        create_dynamic_textures(data.resources.textures);
        NOVA_LOG(DEBUG) << "Dynamic textures created";

        create_render_passes(data.passes, data.pipelines, data.materials);
        NOVA_LOG(DEBUG) << "Created render passes";

        shaderpack_loaded = true;

        NOVA_LOG(INFO) << "Shaderpack " << shaderpack_name << " loaded successfully";
    }

    void NovaRenderer::create_dynamic_textures(const std::vector<shaderpack::TextureCreateInfo>& texture_create_infos) {
        for(const shaderpack::TextureCreateInfo& create_info : texture_create_infos) {
            rhi::Resource* new_texture = rhi->create_texture(create_info);
            dynamic_textures.emplace(create_info.name, new_texture);
        }
    }

    void NovaRenderer::create_render_passes(const std::vector<shaderpack::RenderPassCreateInfo>& pass_create_infos,
                                            const std::vector<shaderpack::PipelineCreateInfo>& pipelines,
                                            const std::vector<shaderpack::MaterialData>& materials) {
        rhi->set_num_renderpasses(static_cast<uint32_t>(pass_create_infos.size()));

        uint32_t total_num_descriptors = 0;
        for(const shaderpack::MaterialData& material_data : materials) {
            for(const shaderpack::MaterialPass& material_pass : material_data.passes) {
                total_num_descriptors += material_pass.bindings.size();
            }
        }

        const rhi::DescriptorPool* descriptor_pool = rhi->create_descriptor_pool(total_num_descriptors, 5, total_num_descriptors);

        for(const shaderpack::RenderPassCreateInfo& create_info : pass_create_infos) {
            Renderpass renderpass;
            RenderpassMetadata metadata;
            metadata.data = create_info;

            Result<rhi::Renderpass*> renderpass_result = rhi->create_renderpass(create_info);
            if(!renderpass_result.has_value) {
                NOVA_LOG(ERROR) << "Could not create renderpass " << create_info.name << ": " << renderpass_result.error.to_string();
            }

            renderpass.renderpass = renderpass_result.value;

            std::vector<rhi::Image*> output_images;
            output_images.reserve(create_info.texture_outputs.size());

            glm::uvec2 framebuffer_size(0);

            std::vector<std::string> attachment_errors;
            attachment_errors.reserve(create_info.texture_outputs.size());

            for(const shaderpack::TextureAttachmentInfo& attachment_info : create_info.texture_outputs) {
                if(attachment_info.name == "Backbuffer") {
                    if(create_info.texture_outputs.size() == 1) {
                        renderpass.writes_to_backbuffer = true;
                        renderpass.framebuffer = nullptr; // Will be resolved when rendering

                    } else {
                        attachment_errors.push_back(fmt::format(
                            fmt("Pass {:s} writes to the backbuffer and {:d} other textures, but that's not allowed. If a pass writes to the backbuffer, it can't write to any other textures"),
                            create_info.name,
                            create_info.texture_outputs.size() - 1));
                    }

                } else {
                    rhi::Image* image = dynamic_textures.at(attachment_info.name);
                    output_images.push_back(image);

                    const shaderpack::TextureCreateInfo& info = dynamic_texture_infos.at(attachment_info.name);
                    const glm::uvec2 attachment_size = info.format.get_size_in_pixels(
                        {render_settings.window.width, render_settings.window.height});

                    if(framebuffer_size.x > 0) {
                        if(attachment_size.x != framebuffer_size.x || attachment_size.y != framebuffer_size.y) {
                            attachment_errors.push_back(fmt::format(
                                fmt("Attachment {:s} has a size of {:d}x{:d}, but the framebuffer for pass {:s} has a size of {:d}x{:d} - these must match! All attachments of a single renderpass must have the same size"),
                                attachment_info.name,
                                attachment_size.x,
                                attachment_size.y,
                                create_info.name,
                                framebuffer_size.x,
                                framebuffer_size.y));
                        }

                    } else {
                        framebuffer_size = attachment_size;
                    }
                }
            }

            if(!attachment_errors.empty()) {
                for(const std::string& err : attachment_errors) {
                    NOVA_LOG(ERROR) << err;
                }
                rhi->destroy_renderpass(renderpass.renderpass);

                NOVA_LOG(ERROR) << "Could not create renderpass " << create_info.name
                                << " because there were errors in the attachment specification. Look above this message for details";
                continue;
            }

            renderpass.framebuffer = rhi->create_framebuffer(renderpass.renderpass, output_images, framebuffer_size);

            renderpass.pipelines.reserve(pipelines.size());
            for(const shaderpack::PipelineCreateInfo& pipeline_create_info : pipelines) {
                if(pipeline_create_info.pass == create_info.name) {
                    std::unordered_map<std::string, ResourceBinding> bindings;

                    Result<rhi::PipelineInterface*> pipeline_interface = create_pipeline_interface(pipeline_create_info,
                                                                                                   create_info.texture_outputs,
                                                                                                   create_info.depth_texture);
                    if(!pipeline_interface) {
                        NOVA_LOG(ERROR) << "Pipeline " << create_info.name
                                        << " has an invalid interface: " << pipeline_interface.error.to_string();
                        continue;
                    }

                    Result<PipelineReturn> pipeline_result = create_graphics_pipeline(pipeline_interface.value,
                                                                                      materials,
                                                                                      pipeline_create_info,
                                                                                      descriptor_pool,
                                                                                      renderpasses.size(),
                                                                                      renderpass.pipelines.size());
                    if(pipeline_result) {
                        auto [pipeline, pipeline_metadata] = *pipeline_result;
                        renderpass.pipelines.push_back(pipeline);

                        metadata.pipeline_metadata.emplace(pipeline_create_info.name, pipeline_metadata);

                    } else {
                        NOVA_LOG(ERROR) << "Could not create pipeline " << pipeline_create_info.name << ": "
                                        << pipeline_result.error.to_string();
                    }
                }
            }
        }
    }

    Result<rhi::PipelineInterface*> NovaRenderer::create_pipeline_interface(
        const shaderpack::PipelineCreateInfo& pipeline_create_info,
        const std::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
        const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) const {
        std::unordered_map<std::string, rhi::ResourceBindingDescription> bindings;
        bindings.reserve(32); // Probably a good estimate

        get_shader_module_descriptors(pipeline_create_info.vertex_shader.source, rhi::ShaderStageFlags::Vertex, bindings);

        if(pipeline_create_info.tessellation_control_shader) {
            get_shader_module_descriptors(pipeline_create_info.tessellation_control_shader->source,
                                          rhi::ShaderStageFlags::TessellationControl,
                                          bindings);
        }
        if(pipeline_create_info.tessellation_evaluation_shader) {
            get_shader_module_descriptors(pipeline_create_info.tessellation_evaluation_shader->source,
                                          rhi::ShaderStageFlags::TessellationEvaluation,
                                          bindings);
        }
        if(pipeline_create_info.geometry_shader) {
            get_shader_module_descriptors(pipeline_create_info.geometry_shader->source, rhi::ShaderStageFlags::Geometry, bindings);
        }
        if(pipeline_create_info.fragment_shader) {
            get_shader_module_descriptors(pipeline_create_info.fragment_shader->source, rhi::ShaderStageFlags::Fragment, bindings);
        }

        return rhi->create_pipeline_interface(bindings, color_attachments, depth_texture);
    }

    Result<NovaRenderer::PipelineReturn> NovaRenderer::create_graphics_pipeline(const rhi::PipelineInterface* pipeline_interface,
                                                                                const std::vector<shaderpack::MaterialData>& materials,
                                                                                const shaderpack::PipelineCreateInfo& pipeline_create_info,
                                                                                const rhi::DescriptorPool* descriptor_pool,
                                                                                const uint32_t renderpass_index,
                                                                                const uint32_t pipeline_index) {
        Pipeline pipeline;
        PipelineMetadata metadata;

        metadata.data = pipeline_create_info;

        Result<rhi::Pipeline*> rhi_pipeline = rhi->create_pipeline(pipeline_interface, pipeline_create_info);
        if(rhi_pipeline) {
            pipeline.pipeline = *rhi_pipeline;

        } else {
            NovaError error = NovaError(fmt::format(fmt("Could not create pipeline {:s}"), pipeline_create_info.name), rhi_pipeline.error);
            return Result<PipelineReturn>(std::move(error));
        }

        // Determine the pipeline layout so the material can create descriptors for the pipeline

        // Large overestimate, but that's fine
        pipeline.passes.reserve(materials.size());

        for(const shaderpack::MaterialData& material_data : materials) {
            for(const shaderpack::MaterialPass& pass_data : material_data.passes) {
                if(pass_data.pipeline == pipeline_create_info.name) {
                    MaterialPass pass = {};

                    pass.descriptor_sets = rhi->create_descriptor_sets(pipeline_interface, descriptor_pool);
                    pass.bindings = pass_data.bindings;

                    FullMaterialPassName full_pass_name = {pass_data.material_name, pass_data.name};

                    MaterialPassMetadata pass_metadata = {};
                    pass_metadata.data = pass_data;
                    metadata.material_metadatas.emplace(full_pass_name, pass_metadata);

                    MaterialPassKey key = {};
                    key.renderpass_index = renderpass_index;
                    key.pipeline_index = pipeline_index;
                    key.material_pass_index = pipeline.passes.size();

                    material_pass_keys.emplace(full_pass_name, key);

                    pipeline.passes.push_back(pass);
                }
            }
        }

        return Result(PipelineReturn{pipeline, metadata});
    }

    void NovaRenderer::get_shader_module_descriptors(const std::vector<uint32_t>& spirv,
                                                     const rhi::ShaderStageFlags shader_stage,
                                                     std::unordered_map<std::string, rhi::ResourceBindingDescription>& bindings) {
        const spirv_cross::CompilerGLSL shader_compiler(spirv);
        const spirv_cross::ShaderResources resources = shader_compiler.get_shader_resources();

        for(const spirv_cross::Resource& resource : resources.sampled_images) {
            NOVA_LOG(TRACE) << "Found a texture resource named " << resource.name;
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, rhi::DescriptorType::CombinedImageSampler);
        }

        for(const spirv_cross::Resource& resource : resources.uniform_buffers) {
            NOVA_LOG(TRACE) << "Found a UBO resource named " << resource.name;
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, rhi::DescriptorType::UniformBuffer);
        }

        for(const spirv_cross::Resource& resource : resources.storage_buffers) {
            NOVA_LOG(TRACE) << "Found a SSBO resource named " << resource.name;
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, rhi::DescriptorType::StorageBuffer);
        }
    }

    void NovaRenderer::add_resource_to_bindings(std::unordered_map<std::string, rhi::ResourceBindingDescription>& bindings,
                                                const rhi::ShaderStageFlags shader_stage,
                                                const spirv_cross::CompilerGLSL& shader_compiler,
                                                const spirv_cross::Resource& resource,
                                                const rhi::DescriptorType type) {
        const uint32_t set = shader_compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        const uint32_t binding = shader_compiler.get_decoration(resource.id, spv::DecorationBinding);

        rhi::ResourceBindingDescription new_binding = {};
        new_binding.set = set;
        new_binding.binding = binding;
        new_binding.type = type;
        new_binding.count = 1;
        new_binding.stages = shader_stage;

        if(bindings.find(resource.name) == bindings.end()) {
            // Totally new binding!
            bindings[resource.name] = new_binding;
        } else {
            // Existing binding. Is it the same as our binding?
            rhi::ResourceBindingDescription& existing_binding = bindings.at(resource.name);
            if(existing_binding != new_binding) {
                // They have two different bindings with the same name. Not allowed
                NOVA_LOG(ERROR) << "You have two different uniforms named " << resource.name
                                << " in different shader stages. This is not allowed. Use unique names";

            } else {
                // Same binding, probably at different stages - let's fix that
                existing_binding.stages |= shader_stage;
            }
        }
    }

    void NovaRenderer::destroy_render_passes() {
        for(Renderpass& renderpass : renderpasses) {
            rhi->destroy_renderpass(renderpass.renderpass);
            rhi->destroy_framebuffer(renderpass.framebuffer);

            for(Pipeline& pipeline : renderpass.pipelines) {
                rhi->destroy_pipeline(pipeline.pipeline);

                for(MaterialPass& material_pass : pipeline.passes) {
                    // TODO: Destroy descriptors for material
                    // TODO: Have a way to save mesh data somewhere outside of the render graph, then process it cleanly here
                }
            }
        }

        renderpasses.clear();
    }

    void NovaRenderer::destroy_dynamic_resources() {
        for(auto& [name, image] : dynamic_textures) {
            rhi->destroy_texture(image);
        }

        dynamic_textures.clear();

        // TODO: Also destroy dynamic buffers, when we have support for those
    }

    rhi::RenderEngine* NovaRenderer::get_engine() const { return rhi.get(); }

    NovaRenderer* NovaRenderer::get_instance() { return instance.get(); }

    NovaRenderer* NovaRenderer::initialize(const NovaSettings& settings) {
        return (instance = std::make_unique<NovaRenderer>(settings)).get();
    }

    void NovaRenderer::deinitialize() { instance.reset(); }
} // namespace nova::renderer
