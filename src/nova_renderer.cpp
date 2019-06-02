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
#include "render_objects/uniform_structs.hpp"
#include "util/logger.hpp"

using namespace foundational::allocation;
using namespace foundational::allocation::operators;

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

        create_global_gpu_pools();

        create_global_sync_objects();

        create_uniform_buffers();
    }

    NovaRenderer::~NovaRenderer() { mtr_shutdown(); }

    NovaSettings& NovaRenderer::get_settings() { return render_settings; }

    void NovaRenderer::execute_frame() {
        MTR_SCOPE("RenderLoop", "execute_frame");
        frame_count++;
        cur_frame_idx = frame_count % 3;

        NOVA_LOG(DEBUG) << "\n***********************\n        FRAME START        \n***********************";

        rhi->wait_for_fences({frame_fences.at(cur_frame_idx)});

        rhi::CommandList* cmds = rhi->get_command_list(0, rhi::QueueType::Graphics);

        for(const Renderpass& renderpass : renderpasses) {
            record_renderpass(renderpass, cmds);
        }

        mtr_flush();
    }

    void NovaRenderer::set_num_meshes(const uint32_t num_meshes) { meshes.reserve(num_meshes); }

    MeshId NovaRenderer::create_mesh(const MeshData& mesh_data) {
        rhi::BufferCreateInfo vertex_buffer_create_info = {};
        vertex_buffer_create_info.buffer_usage = rhi::BufferCreateInfo::Usage::VertexBuffer;
        vertex_buffer_create_info.size = mesh_data.vertex_data.size() * sizeof(FullVertex);
        vertex_buffer_create_info.buffer_residency = rhi::BufferCreateInfo::Residency::DeviceLocal;
        vertex_buffer_create_info.allocation = mesh_memory->allocate(Bytes(vertex_buffer_create_info.size));

        rhi::Buffer* vertex_buffer = rhi->create_buffer(vertex_buffer_create_info);

        {
            rhi::BufferCreateInfo staging_vertex_buffer_create_info = vertex_buffer_create_info;
            staging_vertex_buffer_create_info.buffer_usage = rhi::BufferCreateInfo::Usage::StagingBuffer;
            rhi::Buffer* staging_vertex_buffer = rhi->create_buffer(staging_vertex_buffer_create_info);
            rhi->write_data_to_buffer(mesh_data.vertex_data.data(), mesh_data.vertex_data.size(), staging_vertex_buffer);

            rhi::CommandList* vertex_upload_cmds = rhi->get_command_list(0, rhi::QueueType::Transfer);
            vertex_upload_cmds->copy_buffer(vertex_buffer, 0, staging_vertex_buffer, 0, vertex_buffer_create_info.size);
            rhi->submit_command_list(vertex_upload_cmds, rhi::QueueType::Transfer);
        }

        rhi::BufferCreateInfo index_buffer_create_info = {};
        index_buffer_create_info.buffer_usage = rhi::BufferCreateInfo::Usage::IndexBuffer;
        index_buffer_create_info.size = mesh_data.indices.size() * sizeof(uint32_t);
        index_buffer_create_info.buffer_residency = rhi::BufferCreateInfo::Residency::DeviceLocal;
        index_buffer_create_info.allocation = mesh_memory->allocate(Bytes(index_buffer_create_info.size));

        rhi::Buffer* index_buffer = rhi->create_buffer(index_buffer_create_info);

        {
            rhi::BufferCreateInfo staging_index_buffer_create_info = index_buffer_create_info;
            staging_index_buffer_create_info.buffer_usage = rhi::BufferCreateInfo::Usage::StagingBuffer;
            rhi::Buffer* staging_index_buffer = rhi->create_buffer(staging_index_buffer_create_info);
            rhi->write_data_to_buffer(mesh_data.indices.data(), mesh_data.indices.size(), staging_index_buffer);

            rhi::CommandList* indices_upload_cmds = rhi->get_command_list(0, rhi::QueueType::Transfer);
            indices_upload_cmds->copy_buffer(index_buffer, 0, staging_index_buffer, 0, index_buffer_create_info.size);
            rhi->submit_command_list(indices_upload_cmds, rhi::QueueType::Transfer);
        }

        Mesh mesh = {};
        mesh.vertex_buffer = vertex_buffer;
        mesh.index_buffer = index_buffer;
        mesh.num_indices = mesh_data.indices.size();

        MeshId new_mesh_id = next_mesh_id;
        next_mesh_id++;
        meshes.emplace(new_mesh_id, mesh);

        return new_mesh_id;
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
                total_num_descriptors += static_cast<uint32_t>(material_pass.bindings.size());
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

                    Result<PipelineReturn> pipeline_result = create_graphics_pipeline(*pipeline_interface, pipeline_create_info);
                    if(pipeline_result) {
                        auto [pipeline, pipeline_metadata] = *pipeline_result;

                        MaterialPassKey template_key = {};
                        template_key.renderpass_index = static_cast<uint32_t>(renderpasses.size());
                        template_key.pipeline_index = static_cast<uint32_t>(renderpass.pipelines.size());

                        create_materials_for_pipeline(pipeline,
                                                      pipeline_metadata.material_metadatas,
                                                      materials,
                                                      pipeline_create_info.name,
                                                      *pipeline_interface,
                                                      descriptor_pool,
                                                      template_key);

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

    void NovaRenderer::create_materials_for_pipeline(Pipeline& pipeline,
                                                     std::unordered_map<FullMaterialPassName, MaterialPassMetadata>& material_metadatas,
                                                     const std::vector<shaderpack::MaterialData>& materials,
                                                     const std::string& pipeline_name,
                                                     const rhi::PipelineInterface* pipeline_interface,
                                                     const rhi::DescriptorPool* descriptor_pool,
                                                     const MaterialPassKey& template_key) {

        // Determine the pipeline layout so the material can create descriptors for the pipeline

        // Large overestimate, but that's fine
        pipeline.passes.reserve(materials.size());

        for(const shaderpack::MaterialData& material_data : materials) {
            for(const shaderpack::MaterialPass& pass_data : material_data.passes) {
                if(pass_data.pipeline == pipeline_name) {
                    MaterialPass pass = {};

                    pass.descriptor_sets = rhi->create_descriptor_sets(pipeline_interface, descriptor_pool);

                    bind_data_to_material_descriptor_sets(pass, pass_data.bindings, pipeline_interface->bindings);

                    FullMaterialPassName full_pass_name = {pass_data.material_name, pass_data.name};

                    MaterialPassMetadata pass_metadata = {};
                    pass_metadata.data = pass_data;
                    material_metadatas.emplace(full_pass_name, pass_metadata);

                    MaterialPassKey key = template_key;
                    key.material_pass_index = static_cast<uint32_t>(pipeline.passes.size());

                    material_pass_keys.emplace(full_pass_name, key);

                    pipeline.passes.push_back(pass);
                }
            }
        }
    }

    void NovaRenderer::bind_data_to_material_descriptor_sets(
        const MaterialPass& material,
        const std::unordered_map<std::string, std::string>& bindings,
        const std::unordered_map<std::string, rhi::ResourceBindingDescription>& descriptor_descriptions) {

        std::vector<rhi::DescriptorSetWrite> writes;
        writes.reserve(bindings.size());

        std::vector<rhi::DescriptorImageUpdate> image_updates;
        image_updates.reserve(bindings.size());

        for(const auto& [descriptor_name, resource_name] : bindings) {
            const rhi::ResourceBindingDescription& binding_desc = descriptor_descriptions.at(descriptor_name);
            const rhi::DescriptorSet* descriptor_set = material.descriptor_sets.at(binding_desc.set);

            rhi::DescriptorSetWrite write;
            write.set = descriptor_set;
            write.binding = binding_desc.binding;

            bool is_known = true;
            if(dynamic_textures.find(resource_name) != dynamic_textures.end()) {
                const rhi::Image* image = dynamic_textures.at(resource_name);

                rhi::DescriptorImageUpdate image_update = {};
                image_update.image = image;
                image_update.sampler = point_sampler;
                image_update.format = dynamic_texture_infos.at(resource_name).format;

                image_updates.push_back(image_update);

                write.image_info = &(*image_updates.end());
                write.type = rhi::DescriptorType::CombinedImageSampler;

                writes.push_back(write);

            } else {
                is_known = false;
            }

            if(!is_known) {
                NOVA_LOG(ERROR) << "Resource " << resource_name << " is not known to Nova";
            }
        }

        rhi->update_descriptor_sets(writes);
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

    Result<NovaRenderer::PipelineReturn> NovaRenderer::create_graphics_pipeline(
        rhi::PipelineInterface* pipeline_interface, const shaderpack::PipelineCreateInfo& pipeline_create_info) const {
        Pipeline pipeline;
        PipelineMetadata metadata;

        metadata.data = pipeline_create_info;

        Result<rhi::Pipeline*> rhi_pipeline = rhi->create_pipeline(pipeline_interface, pipeline_create_info);
        if(rhi_pipeline) {
            pipeline.pipeline = *rhi_pipeline;

        } else {
            NovaError error = NovaError(fmt::format(fmt("Could not create pipeline {:s}"), pipeline_create_info.name),
                                        std::move(rhi_pipeline.error));
            return Result<PipelineReturn>(std::move(error));
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

    void NovaRenderer::record_renderpass(Renderpass& renderpass, rhi::CommandList* cmds) {
        cmds->begin_renderpass(renderpass.renderpass, renderpass.framebuffer);

        if(!renderpass.read_texture_barriers.empty()) {
            cmds->resource_barriers(rhi::PipelineStageFlags::ColorAttachmentOutput,
                                    rhi::PipelineStageFlags::FragmentShader,
                                    renderpass.read_texture_barriers);
        }

        if(!renderpass.write_texture_barriers.empty()) {
            cmds->resource_barriers(rhi::PipelineStageFlags::ColorAttachmentOutput,
                                    rhi::PipelineStageFlags::FragmentShader,
                                    renderpass.write_texture_barriers);
        }

        if(renderpass.writes_to_backbuffer) {
            rhi::ResourceBarrier backbuffer_barrier = {};
            backbuffer_barrier.resource_to_barrier = rhi->get_swapchain_image(cur_frame_idx);
            backbuffer_barrier.initial_state = rhi::ResourceState::PresentSource;
            backbuffer_barrier.final_state = rhi::ResourceState::ColorAttachment;
            backbuffer_barrier.access_before_barrier = rhi::ResourceAccessFlags::ColorAttachmentWriteBit;
            backbuffer_barrier.access_after_barrier = rhi::ResourceAccessFlags::ColorAttachmentWriteBit;
            backbuffer_barrier.source_queue = rhi::QueueType::Graphics;
            backbuffer_barrier.destination_queue = rhi::QueueType::Graphics;
            backbuffer_barrier.image_memory_barrier.aspect = rhi::ImageAspectFlags::Color;

            cmds->resource_barriers(rhi::PipelineStageFlags::BottomOfPipe,
                                    rhi::PipelineStageFlags::ColorAttachmentOutput,
                                    {backbuffer_barrier});
        }

        cmds->begin_renderpass(renderpass.renderpass, renderpass.framebuffer);

        for(Pipeline& pipeline : renderpass.pipelines) {
            record_pipeline(pipeline, cmds);
        }

        cmds->end_renderpass();

        if(renderpass.writes_to_backbuffer) {
            rhi::ResourceBarrier backbuffer_barrier = {};
            backbuffer_barrier.resource_to_barrier = rhi->get_swapchain_image(cur_frame_idx);
            backbuffer_barrier.initial_state = rhi::ResourceState::ColorAttachment;
            backbuffer_barrier.final_state = rhi::ResourceState::PresentSource;
            backbuffer_barrier.access_before_barrier = rhi::ResourceAccessFlags::ColorAttachmentWriteBit;
            backbuffer_barrier.access_after_barrier = rhi::ResourceAccessFlags::ColorAttachmentWriteBit;
            backbuffer_barrier.source_queue = rhi::QueueType::Graphics;
            backbuffer_barrier.destination_queue = rhi::QueueType::Graphics;
            backbuffer_barrier.image_memory_barrier.aspect = rhi::ImageAspectFlags::Color;

            cmds->resource_barriers(rhi::PipelineStageFlags::ColorAttachmentOutput,
                                    rhi::PipelineStageFlags::BottomOfPipe,
                                    {backbuffer_barrier});
        }
    }

    void NovaRenderer::record_pipeline(Pipeline& pipeline, rhi::CommandList* cmds) {
        cmds->bind_pipeline(pipeline.pipeline);

        for(MaterialPass& pass : pipeline.passes) {
            record_material_pass(pass, cmds);
        }
    }

    void NovaRenderer::record_material_pass(MaterialPass& pass, rhi::CommandList* cmds) {
        cmds->bind_descriptor_sets(pass.descriptor_sets, pass.pipeline_interface);

        for(MeshBatch<StaticMeshRenderCommand>& batch : pass.static_mesh_draws) {
            record_rendering_static_mesh_batch(batch, cmds);
        }
    }

    void NovaRenderer::record_rendering_static_mesh_batch(MeshBatch<StaticMeshRenderCommand>& batch, rhi::CommandList* cmds) {
        const std::vector<rhi::Buffer*> vertex_buffers = {batch.vertex_buffer,
                                                          batch.vertex_buffer,
                                                          batch.vertex_buffer,
                                                          batch.vertex_buffer,
                                                          batch.vertex_buffer,
                                                          batch.vertex_buffer,
                                                          batch.vertex_buffer};
        cmds->bind_vertex_buffers(vertex_buffers);
        cmds->bind_index_buffer(batch.index_buffer);

        const uint64_t start_index = cur_model_matrix_index;

        for(const StaticMeshRenderCommand& command : batch.renderables) {
            if(command.is_visible) {
                rhi->write_data_to_buffer(&command.model_matrix, sizeof(glm::mat4), );
            }
        }
    }

    RenderableId NovaRenderer::add_renderable_for_material(const FullMaterialPassName& material_name,
                                                           const StaticMeshRenderableData& renderable) {
        const RenderableId id = next_renderable_id.load();
        next_renderable_id.fetch_add(1);

        // Figure out where to put the renderable
        const MaterialPassKey& pass_key = material_pass_keys.at(material_name);

        Renderpass& renderpass = renderpasses.at(pass_key.renderpass_index);
        Pipeline& pipeline = renderpass.pipelines.at(pass_key.pipeline_index);
        MaterialPass& material = pipeline.passes.at(pass_key.material_pass_index);

        const Mesh& mesh = meshes.at(renderable.mesh);

        if(renderable.is_static) {
            for(MeshBatch<StaticMeshRenderCommand>& batch : material.static_mesh_draws) {
                if(batch.vertex_buffer == mesh.vertex_buffer) {
                    StaticMeshRenderCommand command = {};
                    command.id = id;
                    command.is_visible = true;
                    // TODO: Make sure this is accurate
                    command.model_matrix = renderable.initial_position * renderable.initial_rotation * renderable.initial_scale;

                    batch.renderables.emplace_back(command);
                }
            }
        }

        return id;
    }

    rhi::RenderEngine* NovaRenderer::get_engine() const { return rhi.get(); }

    NovaRenderer* NovaRenderer::get_instance() { return instance.get(); }

    NovaRenderer* NovaRenderer::initialize(const NovaSettings& settings) {
        return (instance = std::make_unique<NovaRenderer>(settings)).get();
    }

    void NovaRenderer::deinitialize() { instance.reset(); }

    void NovaRenderer::create_global_gpu_pools() {
        const uint64_t mesh_memory_size = 512000000;
        Result<std::unique_ptr<DeviceMemoryResource>>
            mesh_memory_result = rhi->allocate_device_memory(mesh_memory_size, rhi::MemoryUsage::DeviceOnly, rhi::ObjectType::Buffer)
                                     .map([&](rhi::DeviceMemory* memory) {
                                         auto* allocator = new BlockAllocator(nullptr, Bytes(mesh_memory_size), 64_b);
                                         return std::make_unique<DeviceMemoryResource>(memory, allocator);
                                     });

        if(mesh_memory_result) {
            mesh_memory = std::move(mesh_memory_result.value);

        } else {
            NOVA_LOG(ERROR) << "Could not create mesh memory pool: " << mesh_memory_result.error.to_string();
        }

        // Assume 65k things, plus we need space for the builtin ubos
        const uint64_t ubo_memory_size = sizeof(PerFrameUniforms) + sizeof(glm::mat4) * 0xFFFF;
        Result<std::unique_ptr<DeviceMemoryResource>>
            ubo_memory_result = rhi->allocate_device_memory(ubo_memory_size, rhi::MemoryUsage::DeviceOnly, rhi::ObjectType::Buffer)
                                    .map([&](rhi::DeviceMemory* memory) {
                                        auto* allocator = new BumpPointAllocator(Bytes(ubo_memory_size), Bytes(sizeof(glm::mat4)));
                                        return std::make_unique<DeviceMemoryResource>(memory, allocator);
                                    });

        if(ubo_memory_result) {
            ubo_memory = std::move(ubo_memory_result.value);

        } else {
            NOVA_LOG(ERROR) << "Could not create mesh memory pool: " << ubo_memory_result.error.to_string();
        }

        // Staging buffers will be pooled, so we don't need a _ton_ of memory for them
        const Bytes staging_memory_size = 256_kb;
        Result<std::unique_ptr<DeviceMemoryResource>>
            staging_memory_result = rhi->allocate_device_memory(staging_memory_size.b_count(),
                                                                rhi::MemoryUsage::StagingBuffer,
                                                                rhi::ObjectType::Buffer)
                                        .map([&](rhi::DeviceMemory* memory) {
                                            auto* allocator = new BumpPointAllocator(staging_memory_size, 64_b);
                                            return std::make_unique<DeviceMemoryResource>(memory, allocator);
                                        });

        if(staging_memory_result) {
            staging_buffer_memory = std::move(staging_memory_result.value);

        } else {
            NOVA_LOG(ERROR) << "Could not create staging buffer memory pool: " << staging_memory_result.error.to_string();
        }
    }

    void NovaRenderer::create_global_sync_objects() {
        const std::vector<rhi::Fence*>& fences = rhi->create_fences(NUM_IN_FLIGHT_FRAMES, true);
        for(uint32_t i = 0; i < NUM_IN_FLIGHT_FRAMES; i++) {
            frame_fences[i] = fences.at(i);
        }
    }

    void NovaRenderer::create_uniform_buffers() {
        rhi::BufferCreateInfo per_frame_data_create_info = {};
        per_frame_data_create_info.size = sizeof(PerFrameUniforms);
        per_frame_data_create_info.buffer_usage = rhi::BufferCreateInfo::Usage::UniformBuffer;
        per_frame_data_create_info.allocation = ubo_memory->allocate(Bytes(sizeof(PerFrameUniforms)));

        per_frame_data_buffer = rhi->create_buffer(per_frame_data_create_info);

        rhi::BufferCreateInfo model_matrix_buffer_create_info = {};
        model_matrix_buffer_create_info.size = sizeof(glm::mat4) * 0xFFFF;
        model_matrix_buffer_create_info.buffer_usage = rhi::BufferCreateInfo::Usage::UniformBuffer;
        model_matrix_buffer_create_info.allocation = ubo_memory->allocate(Bytes(sizeof(glm::mat4) * 0xFFFF));

        model_matrix_buffer = rhi->create_buffer(model_matrix_buffer_create_info);
    }
} // namespace nova::renderer
