#include "nova_renderer/nova_renderer.hpp"

#include <array>
#include <future>

#pragma warning(push, 0)
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glslang/MachineIndependent/Initialize.h>
#include <minitrace.h>
#include <spirv_glsl.hpp>
#pragma warning(pop)

#include "nova_renderer/command_list.hpp"
#include "nova_renderer/constants.hpp"
#include "nova_renderer/frontend/procedural_mesh.hpp"
#include "nova_renderer/swapchain.hpp"
#include "nova_renderer/util/platform.hpp"

#include "debugging/renderdoc.hpp"
#include "loading/shaderpack/render_graph_builder.hpp"
#include "loading/shaderpack/shaderpack_loading.hpp"
#include "memory/block_allocation_strategy.hpp"
#include "memory/bump_point_allocation_strategy.hpp"
#include "memory/mallocator.hpp"
#include "memory/system_memory_allocator.hpp"
#include "render_objects/uniform_structs.hpp"
// D3D12 MUST be included first because the Vulkan include undefines FAR, yet the D3D12 headers need FAR
// Windows considered harmful
#if defined(NOVA_WINDOWS) && defined(NOVA_D3D12_RHI)
#include "render_engine/dx12/dx12_render_engine.hpp"
#endif

#if defined(NOVA_VULKAN_RHI)
#include "render_engine/vulkan/vulkan_render_engine.hpp"
#endif
#if defined(NOVA_OPENGL_RHI)
#include "render_engine/gl3/gl3_render_engine.hpp"
#endif

#include "util/logger.hpp"

using namespace bvestl::polyalloc;
using namespace bvestl::polyalloc::operators;

const Bytes global_memory_pool_size = 1_gb;

namespace nova::renderer {
    std::unique_ptr<NovaRenderer> NovaRenderer::instance;

    bool FullMaterialPassName::operator==(const FullMaterialPassName& other) const {
        return material_name == other.material_name && pass_name == other.pass_name;
    }

    std::size_t FullMaterialPassNameHasher::operator()(const FullMaterialPassName& name) const {
        const std::size_t material_name_hash = std::hash<std::string>()(name.material_name);
        const std::size_t pass_name_hash = std::hash<std::string>()(name.pass_name);

        return material_name_hash ^ pass_name_hash;
    }

    NovaRenderer::NovaRenderer(const NovaSettings& settings) : render_settings(settings) {
        create_global_allocator();

        mtr_init("trace.json");

        MTR_META_PROCESS_NAME("NovaRenderer");
        MTR_META_THREAD_NAME("Main");

        MTR_SCOPE("Init", "nova_renderer::nova_renderer");

        window = std::make_shared<NovaWindow>(settings);

        if(settings.debug.renderdoc.enabled) {
            MTR_SCOPE("Init", "LoadRenderdoc");
            auto rd_load_result = load_renderdoc(settings.debug.renderdoc.renderdoc_dll_path);

            rd_load_result
                .map([&](RENDERDOC_API_1_3_0* api) {
                    render_doc = api;

                    render_doc->SetCaptureFilePathTemplate(settings.debug.renderdoc.capture_path);

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
                .on_error([](const ntl::NovaError& error) { NOVA_LOG(ERROR) << error.to_string().c_str(); });
        }

        switch(settings.api) {
            case GraphicsApi::D3D12:
#if defined(NOVA_WINDOWS) && defined(NOVA_D3D12_RHI)
            {
                MTR_SCOPE("Init", "InitDirect3D12RenderEngine");
                rhi = std::make_unique<rhi::D3D12RenderEngine>(render_settings, window);
            } break;
#endif

#if defined(NOVA_VULKAN_RHI)
            case GraphicsApi::Vulkan: {
                MTR_SCOPE("Init", "InitVulkanRenderEngine");
                rhi = std::make_unique<rhi::VulkanRenderEngine>(render_settings, window);
            } break;
#endif

#if defined(NOVA_OPENGL_RHI)
            case GraphicsApi::NvGl4: {
                MTR_SCOPE("Init", "InitGL3RenderEngine");
                rhi = std::make_unique<rhi::Gl4NvRenderEngine>(render_settings, window);
            } break;
#endif
            default: {
                // TODO: Deal with it in a better way, this will crash soon
                NOVA_LOG(FATAL) << "Selected graphics API was not enabled!";
            } break;
        }

        swapchain = rhi->get_swapchain();

        create_global_gpu_pools();

        create_global_sync_objects();

        create_uniform_buffers();
    }

    NovaRenderer::~NovaRenderer() { mtr_shutdown(); }

    NovaSettingsAccessManager& NovaRenderer::get_settings() { return render_settings; }

    void NovaRenderer::execute_frame() {
        MTR_SCOPE("RenderLoop", "execute_frame");
        frame_count++;
        cur_frame_idx = rhi->get_swapchain()->acquire_next_swapchain_image();

        NOVA_LOG(DEBUG) << "\n***********************\n        FRAME START        \n***********************";

        rhi->reset_fences({frame_fences.at(cur_frame_idx)});

        rhi::CommandList* cmds = rhi->get_command_list(0, rhi::QueueType::Graphics);

        // This may or may not work well lmao
        for(auto& [id, proc_mesh] : proc_meshes) {
            proc_mesh.record_commands_to_upload_data(cmds, cur_frame_idx);
        }

        for(Renderpass& renderpass : renderpasses) {
            record_renderpass(renderpass, cmds);
        }

        rhi->submit_command_list(cmds, rhi::QueueType::Graphics, frame_fences.at(cur_frame_idx));

        // Wait for the GPU to finish before presenting. This destroys pipelining and throughput, however at this time I'm not sure how best
        // to say "when GPU finishes this task, CPU should do something"
        rhi->wait_for_fences({frame_fences.at(cur_frame_idx)});

        rhi->get_swapchain()->present(cur_frame_idx);

        mtr_flush();
    }

    void NovaRenderer::set_num_meshes(const uint32_t num_meshes) { meshes.reserve(num_meshes); }

    MeshId NovaRenderer::create_mesh(const MeshData& mesh_data) {
        rhi::BufferCreateInfo vertex_buffer_create_info;
        vertex_buffer_create_info.buffer_usage = rhi::BufferUsage::VertexBuffer;
        vertex_buffer_create_info.size = mesh_data.vertex_data.size() * sizeof(FullVertex);

        rhi::Buffer* vertex_buffer = rhi->create_buffer(vertex_buffer_create_info, *mesh_memory);

        // TODO: Try to get staging buffers from a pool

        {
            rhi::BufferCreateInfo staging_vertex_buffer_create_info = vertex_buffer_create_info;
            staging_vertex_buffer_create_info.buffer_usage = rhi::BufferUsage::StagingBuffer;

            rhi::Buffer* staging_vertex_buffer = rhi->create_buffer(staging_vertex_buffer_create_info, *staging_buffer_memory);
            rhi->write_data_to_buffer(mesh_data.vertex_data.data(),
                                      mesh_data.vertex_data.size() * sizeof(FullVertex),
                                      0,
                                      staging_vertex_buffer);

            rhi::CommandList* vertex_upload_cmds = rhi->get_command_list(0, rhi::QueueType::Transfer);
            vertex_upload_cmds->copy_buffer(vertex_buffer, 0, staging_vertex_buffer, 0, vertex_buffer_create_info.size);

            rhi::ResourceBarrier vertex_barrier = {};
            vertex_barrier.resource_to_barrier = vertex_buffer;
            vertex_barrier.old_state = rhi::ResourceState::CopyDestination;
            vertex_barrier.new_state = rhi::ResourceState::Common;
            vertex_barrier.access_before_barrier = rhi::AccessFlags::CopyWrite;
            vertex_barrier.access_after_barrier = rhi::AccessFlags::VertexAttributeRead;
            vertex_barrier.buffer_memory_barrier.offset = 0;
            vertex_barrier.buffer_memory_barrier.size = vertex_buffer->size;

            vertex_upload_cmds->resource_barriers(rhi::PipelineStageFlags::Transfer,
                                                  rhi::PipelineStageFlags::VertexInput,
                                                  {vertex_barrier});

            rhi->submit_command_list(vertex_upload_cmds, rhi::QueueType::Transfer);

            // TODO: Barrier on the mesh's first usage
        }

        rhi::BufferCreateInfo index_buffer_create_info;
        index_buffer_create_info.buffer_usage = rhi::BufferUsage::IndexBuffer;
        index_buffer_create_info.size = mesh_data.indices.size() * sizeof(uint32_t);

        rhi::Buffer* index_buffer = rhi->create_buffer(index_buffer_create_info, *mesh_memory);

        {
            rhi::BufferCreateInfo staging_index_buffer_create_info = index_buffer_create_info;
            staging_index_buffer_create_info.buffer_usage = rhi::BufferUsage::StagingBuffer;
            rhi::Buffer* staging_index_buffer = rhi->create_buffer(staging_index_buffer_create_info, *staging_buffer_memory);
            rhi->write_data_to_buffer(mesh_data.indices.data(), mesh_data.indices.size() * sizeof(uint32_t), 0, staging_index_buffer);

            rhi::CommandList* indices_upload_cmds = rhi->get_command_list(0, rhi::QueueType::Transfer);
            indices_upload_cmds->copy_buffer(index_buffer, 0, staging_index_buffer, 0, index_buffer_create_info.size);

            rhi::ResourceBarrier index_barrier = {};
            index_barrier.resource_to_barrier = index_buffer;
            index_barrier.old_state = rhi::ResourceState::CopyDestination;
            index_barrier.new_state = rhi::ResourceState::Common;
            index_barrier.access_before_barrier = rhi::AccessFlags::CopyWrite;
            index_barrier.access_after_barrier = rhi::AccessFlags::IndexRead;
            index_barrier.buffer_memory_barrier.offset = 0;
            index_barrier.buffer_memory_barrier.size = index_buffer->size;

            indices_upload_cmds->resource_barriers(rhi::PipelineStageFlags::Transfer,
                                                   rhi::PipelineStageFlags::VertexInput,
                                                   {index_barrier});

            rhi->submit_command_list(indices_upload_cmds, rhi::QueueType::Transfer);

            // TODO: Barrier on the mesh's first usage
        }

        // TODO: Clean up staging buffers

        Mesh mesh;
        mesh.vertex_buffer = vertex_buffer;
        mesh.index_buffer = index_buffer;
        mesh.num_indices = static_cast<uint32_t>(mesh_data.indices.size());

        MeshId new_mesh_id = next_mesh_id;
        next_mesh_id++;
        meshes.emplace(new_mesh_id, mesh);

        return new_mesh_id;
    }

    MapAccessor<MeshId, ProceduralMesh> NovaRenderer::create_procedural_mesh(const uint64_t vertex_size, const uint64_t index_size) {
        MeshId our_id = next_mesh_id;
        next_mesh_id++;

        proc_meshes.emplace(our_id, ProceduralMesh(vertex_size, index_size, rhi.get()));

        return MapAccessor<MeshId, ProceduralMesh>(&proc_meshes, our_id);
    }

    void NovaRenderer::load_shaderpack(const std::string& shaderpack_name) {
        MTR_SCOPE("ShaderpackLoading", "load_shaderpack");
        glslang::InitializeProcess();

        const shaderpack::ShaderpackData data = shaderpack::load_shaderpack_data(fs::path(shaderpack_name.c_str()));

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

        NOVA_LOG(INFO) << "Shaderpack " << shaderpack_name.c_str() << " loaded successfully";
    }

    void NovaRenderer::create_dynamic_textures(const std::vector<shaderpack::TextureCreateInfo>& texture_create_infos) {
        for(const shaderpack::TextureCreateInfo& create_info : texture_create_infos) {
            rhi::Image* new_texture = rhi->create_image(create_info);
            dynamic_textures.emplace(create_info.name, new_texture);
            dynamic_texture_infos.emplace(create_info.name, create_info);
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

        rhi::DescriptorPool* descriptor_pool = rhi->create_descriptor_pool(total_num_descriptors, 5, total_num_descriptors);

        for(const shaderpack::RenderPassCreateInfo& create_info : pass_create_infos) {
            Renderpass renderpass;
            RenderpassMetadata metadata;
            metadata.data = create_info;

            std::vector<rhi::Image*> color_attachments;
            color_attachments.reserve(create_info.texture_outputs.size());

            glm::uvec2 framebuffer_size(0);

            const auto num_attachments = create_info.depth_texture ? create_info.texture_outputs.size() + 1 :
                                                                     create_info.texture_outputs.size();
            std::vector<std::string> attachment_errors;
            attachment_errors.reserve(num_attachments);

            bool writes_to_backbuffer = false;

            for(const shaderpack::TextureAttachmentInfo& attachment_info : create_info.texture_outputs) {
                if(attachment_info.name == "Backbuffer") {
                    writes_to_backbuffer = true;

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
                    color_attachments.push_back(image);

                    const shaderpack::TextureCreateInfo& info = dynamic_texture_infos.at(attachment_info.name);
                    const glm::uvec2 attachment_size = info.format.get_size_in_pixels(
                        {render_settings.settings.window.width, render_settings.settings.window.height});

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

            // Can't combine these if statements and I don't want to `.find` twice
            const auto depth_attachment = [&]() -> std::optional<rhi::Image*> {
                if(create_info.depth_texture) {
                    if(const auto depth_tex_itr = dynamic_textures.find(create_info.depth_texture->name);
                       depth_tex_itr != dynamic_textures.end()) {
                        auto* image = depth_tex_itr->second;
                        return std::make_optional<rhi::Image*>(image);
                    }
                }

                return {};
            }();

            if(!attachment_errors.empty()) {
                for(const auto& err : attachment_errors) {
                    NOVA_LOG(ERROR) << err;
                }

                NOVA_LOG(ERROR) << "Could not create renderpass " << create_info.name
                                << " because there were errors in the attachment specification. Look above this message for details";
                continue;
            }

            ntl::Result<rhi::Renderpass*> renderpass_result = rhi->create_renderpass(create_info, framebuffer_size);
            if(renderpass_result) {
                renderpass.renderpass = renderpass_result.value;

            } else {
                NOVA_LOG(ERROR) << "Could not create renderpass " << create_info.name << ": " << renderpass_result.error.to_string();
                continue;
            }

            // Backbuffer framebuffers are owned by the swapchain, not the renderpass that writes to them, so if the
            // renderpass writes to the backbuffer then we don't need to create a framebuffer for it
            if(!writes_to_backbuffer) {
                renderpass.framebuffer = rhi->create_framebuffer(renderpass.renderpass,
                                                                 color_attachments,
                                                                 depth_attachment,
                                                                 framebuffer_size);
            }

            renderpass.pipelines.reserve(pipelines.size());
            for(const shaderpack::PipelineCreateInfo& pipeline_create_info : pipelines) {
                if(pipeline_create_info.pass == create_info.name) {
                    std::unordered_map<std::string, ResourceBinding> bindings;

                    ntl::Result<rhi::PipelineInterface*> pipeline_interface = create_pipeline_interface(pipeline_create_info,
                                                                                                        create_info.texture_outputs,
                                                                                                        create_info.depth_texture);
                    if(!pipeline_interface) {
                        NOVA_LOG(ERROR) << "Pipeline " << create_info.name
                                        << " has an invalid interface: " << pipeline_interface.error.to_string();
                        continue;
                    }

                    ntl::Result<PipelineReturn> pipeline_result = create_graphics_pipeline(*pipeline_interface, pipeline_create_info);
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

            renderpass.id = static_cast<uint32_t>(renderpass_metadatas.size());

            renderpasses.push_back(renderpass);
            renderpass_metadatas.push_back(metadata);
        }
    }

    void NovaRenderer::create_materials_for_pipeline(
        Pipeline& pipeline,
        std::unordered_map<FullMaterialPassName, MaterialPassMetadata, FullMaterialPassNameHasher>& material_metadatas,
        const std::vector<shaderpack::MaterialData>& materials,
        const std::string& pipeline_name,
        const rhi::PipelineInterface* pipeline_interface,
        rhi::DescriptorPool* descriptor_pool,
        const MaterialPassKey& template_key) {

        // Determine the pipeline layout so the material can create descriptors for the pipeline

        // Large overestimate, but that's fine
        pipeline.passes.reserve(materials.size());

        for(const shaderpack::MaterialData& material_data : materials) {
            for(const shaderpack::MaterialPass& pass_data : material_data.passes) {
                if(pass_data.pipeline == pipeline_name) {
                    MaterialPass pass = {};
                    pass.pipeline_interface = pipeline_interface;

                    pass.descriptor_sets = rhi->create_descriptor_sets(pipeline_interface, descriptor_pool);

                    bind_data_to_material_descriptor_sets(pass, pass_data.bindings, pipeline_interface->bindings);

                    FullMaterialPassName full_pass_name = {pass_data.material_name, pass_data.name};

                    MaterialPassMetadata pass_metadata{};
                    pass_metadata.data = pass_data;
                    material_metadatas.emplace(full_pass_name, pass_metadata);

                    MaterialPassKey key = template_key;
                    key.material_pass_index = static_cast<uint32_t>(pipeline.passes.size());

                    material_pass_keys.emplace(full_pass_name, key);

                    pipeline.passes.push_back(pass);
                }
            }
        }

        pipeline.passes.shrink_to_fit();
    }

    void NovaRenderer::bind_data_to_material_descriptor_sets(
        const MaterialPass& material,
        const std::unordered_map<std::string, std::string>& bindings,
        const std::unordered_map<std::string, rhi::ResourceBindingDescription>& descriptor_descriptions) {

        std::vector<rhi::DescriptorSetWrite> writes;
        writes.reserve(bindings.size());

        for(const auto& [descriptor_name, resource_name] : bindings) {
            const rhi::ResourceBindingDescription& binding_desc = descriptor_descriptions.at(descriptor_name);
            rhi::DescriptorSet* descriptor_set = material.descriptor_sets.at(binding_desc.set);

            rhi::DescriptorSetWrite write = {};
            write.set = descriptor_set;
            write.first_binding = binding_desc.binding;
            write.resources.emplace_back();
            rhi::DescriptorResourceInfo& resource_info = write.resources[0];

            if(const auto dyn_tex_itr = dynamic_textures.find(resource_name); dyn_tex_itr != dynamic_textures.end()) {
                rhi::Image* image = dyn_tex_itr->second;

                resource_info.image_info.image = image;
                resource_info.image_info.sampler = point_sampler;
                resource_info.image_info.format = dynamic_texture_infos.at(resource_name).format;

                write.type = rhi::DescriptorType::CombinedImageSampler;

                writes.push_back(write);

            } else if(const auto builtin_buffer_itr = builtin_buffers.find(resource_name); builtin_buffer_itr != builtin_buffers.end()) {
                rhi::Buffer* buffer = builtin_buffer_itr->second;

                resource_info.buffer_info.buffer = buffer;
                write.type = rhi::DescriptorType::UniformBuffer;

                writes.push_back(write);

            } else {
                NOVA_LOG(ERROR) << "Resource " << resource_name.c_str() << " is not known to Nova";
            }
        }

        rhi->update_descriptor_sets(writes);
    }

    ntl::Result<rhi::PipelineInterface*> NovaRenderer::create_pipeline_interface(
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

    ntl::Result<NovaRenderer::PipelineReturn> NovaRenderer::create_graphics_pipeline(
        rhi::PipelineInterface* pipeline_interface, const shaderpack::PipelineCreateInfo& pipeline_create_info) const {
        Pipeline pipeline;
        PipelineMetadata metadata;

        metadata.data = pipeline_create_info;

        ntl::Result<rhi::Pipeline*> rhi_pipeline = rhi->create_pipeline(pipeline_interface, pipeline_create_info);
        if(rhi_pipeline) {
            pipeline.pipeline = *rhi_pipeline;

        } else {
            ntl::NovaError error = ntl::NovaError(fmt::format(fmt("Could not create pipeline {:s}"), pipeline_create_info.name.c_str())
                                                      .c_str(),
                                                  std::move(rhi_pipeline.error));
            return ntl::Result<PipelineReturn>(std::move(error));
        }

        return ntl::Result(PipelineReturn{pipeline, metadata});
    }

    void NovaRenderer::get_shader_module_descriptors(const std::vector<uint32_t>& spirv,
                                                     const rhi::ShaderStageFlags shader_stage,
                                                     std::unordered_map<std::string, rhi::ResourceBindingDescription>& bindings) {
        std::vector<uint32_t> spirv_std(spirv.begin(), spirv.end());
        const spirv_cross::CompilerGLSL shader_compiler(spirv_std);
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

        const spirv_cross::SPIRType& type_information = shader_compiler.get_type(resource.type_id);
        if(!type_information.array.empty()) {
            new_binding.count = type_information.array[0];
            // All arrays are unbounded until I figure out how to use SPIRV-Cross to detect unbounded arrays
            new_binding.is_unbounded = true;
        }

        const std::string& resource_name = resource.name;

        if(const auto itr = bindings.find(resource_name); itr != bindings.end()) {
            // Existing binding. Is it the same as our binding?
            rhi::ResourceBindingDescription& existing_binding = itr->second;
            if(existing_binding != new_binding) {
                // They have two different bindings with the same name. Not allowed
                NOVA_LOG(ERROR) << "You have two different uniforms named " << resource.name
                                << " in different shader stages. This is not allowed. Use unique names";

            } else {
                // Same binding, probably at different stages - let's fix that
                existing_binding.stages |= shader_stage;
            }

        } else {
            // Totally new binding!
            bindings[resource_name] = new_binding;
        }
    }

    void NovaRenderer::destroy_render_passes() {
        for(Renderpass& renderpass : renderpasses) {
            rhi->destroy_renderpass(renderpass.renderpass);
            rhi->destroy_framebuffer(renderpass.framebuffer);

            for(Pipeline& pipeline : renderpass.pipelines) {
                rhi->destroy_pipeline(pipeline.pipeline);

                for(MaterialPass& material_pass : pipeline.passes) {
                    (void) material_pass;
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
        // TODO: Figure if any of these barriers are implicit
        // TODO: Use shader reflection to figure our the stage that the pipelines in this renderpass need access to this resource instead of
        // using a robust default

        if(!renderpass.read_texture_barriers.empty()) {
            // TODO: Use shader reflection to figure our the stage that the pipelines in this renderpass need access to this resource
            // instead of using a robust default
            cmds->resource_barriers(rhi::PipelineStageFlags::ColorAttachmentOutput,
                                    rhi::PipelineStageFlags::FragmentShader,
                                    renderpass.read_texture_barriers);
        }

        if(!renderpass.write_texture_barriers.empty()) {
            // TODO: Use shader reflection to figure our the stage that the pipelines in this renderpass need access to this resource
            // instead of using a robust default
            cmds->resource_barriers(rhi::PipelineStageFlags::ColorAttachmentOutput,
                                    rhi::PipelineStageFlags::FragmentShader,
                                    renderpass.write_texture_barriers);
        }

        if(renderpass.writes_to_backbuffer) {
            rhi::ResourceBarrier backbuffer_barrier{};
            backbuffer_barrier.resource_to_barrier = swapchain->get_image(cur_frame_idx);
            backbuffer_barrier.access_before_barrier = rhi::AccessFlags::MemoryRead;
            backbuffer_barrier.access_after_barrier = rhi::AccessFlags::ColorAttachmentWrite;
            backbuffer_barrier.old_state = rhi::ResourceState::PresentSource;
            backbuffer_barrier.new_state = rhi::ResourceState::RenderTarget;
            backbuffer_barrier.source_queue = rhi::QueueType::Graphics;
            backbuffer_barrier.destination_queue = rhi::QueueType::Graphics;
            backbuffer_barrier.image_memory_barrier.aspect = rhi::ImageAspectFlags::Color;

            // TODO: Use shader reflection to figure our the stage that the pipelines in this renderpass need access to this resource
            // instead of using a robust default
            cmds->resource_barriers(rhi::PipelineStageFlags::TopOfPipe,
                                    rhi::PipelineStageFlags::ColorAttachmentOutput,
                                    {backbuffer_barrier});
        }

        const auto& renderpass_metadata = renderpass_metadatas.at(renderpass.id);
        NOVA_LOG(TRACE) << "Beginning renderpass " << renderpass_metadata.data.name;

        const auto framebuffer = [&] {
            if(!renderpass.writes_to_backbuffer) {
                return renderpass.framebuffer;
            } else {
                return swapchain->get_framebuffer(cur_frame_idx);
            }
        }();

        cmds->begin_renderpass(renderpass.renderpass, framebuffer);

        for(Pipeline& pipeline : renderpass.pipelines) {
            record_pipeline(pipeline, cmds);
        }

        NOVA_LOG(TRACE) << "Ending renderpass " << renderpass_metadata.data.name;
        cmds->end_renderpass();

        if(renderpass.writes_to_backbuffer) {
            rhi::ResourceBarrier backbuffer_barrier{};
            backbuffer_barrier.resource_to_barrier = swapchain->get_image(cur_frame_idx);
            backbuffer_barrier.access_before_barrier = rhi::AccessFlags::ColorAttachmentWrite;
            backbuffer_barrier.access_after_barrier = rhi::AccessFlags::MemoryRead;
            backbuffer_barrier.old_state = rhi::ResourceState::RenderTarget;
            backbuffer_barrier.new_state = rhi::ResourceState::PresentSource;
            backbuffer_barrier.source_queue = rhi::QueueType::Graphics;
            backbuffer_barrier.destination_queue = rhi::QueueType::Graphics;
            backbuffer_barrier.image_memory_barrier.aspect = rhi::ImageAspectFlags::Color;

            // When this line executes, the D3D12 debug layer gets mad about "A single command list cannot write to multiple buffers within
            // a particular swapchain" and I don't know why it's mad about that, or even really what that message means
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

        for(const MeshBatch<StaticMeshRenderCommand>& batch : pass.static_mesh_draws) {
            record_rendering_static_mesh_batch(batch, cmds);
        }

        for(const ProceduralMeshBatch<StaticMeshRenderCommand>& batch : pass.static_procedural_mesh_draws) {
            record_rendering_static_mesh_batch(batch, cmds);
        }
    }

    void NovaRenderer::record_rendering_static_mesh_batch(const MeshBatch<StaticMeshRenderCommand>& batch, rhi::CommandList* cmds) {
        const uint32_t start_index = cur_model_matrix_index;

        for(const StaticMeshRenderCommand& command : batch.commands) {
            if(command.is_visible) {
                auto* model_matrix_buffer = builtin_buffers.at(MODEL_MATRIX_BUFFER_NAME);
                rhi->write_data_to_buffer(&command.model_matrix,
                                          sizeof(glm::mat4),
                                          cur_model_matrix_index * sizeof(glm::mat4),
                                          model_matrix_buffer);
                cur_model_matrix_index++;
            }
        }

        if(start_index != cur_model_matrix_index) {
            // TODO: There's probably a better way to do this
            const std::vector<rhi::Buffer*> vertex_buffers = {batch.vertex_buffer,
                                                              batch.vertex_buffer,
                                                              batch.vertex_buffer,
                                                              batch.vertex_buffer,
                                                              batch.vertex_buffer,
                                                              batch.vertex_buffer,
                                                              batch.vertex_buffer};
            cmds->bind_vertex_buffers(vertex_buffers);
            cmds->bind_index_buffer(batch.index_buffer);

            cmds->draw_indexed_mesh(static_cast<uint32_t>(batch.index_buffer->size / sizeof(uint32_t)),
                                    cur_model_matrix_index - start_index);
        }
    }

    void NovaRenderer::record_rendering_static_mesh_batch(const ProceduralMeshBatch<StaticMeshRenderCommand>& batch,
                                                          rhi::CommandList* cmds) {
        const uint32_t start_index = cur_model_matrix_index;

        for(const StaticMeshRenderCommand& command : batch.commands) {
            if(command.is_visible) {
                auto* model_matrix_buffer = builtin_buffers.at(MODEL_MATRIX_BUFFER_NAME);
                rhi->write_data_to_buffer(&command.model_matrix,
                                          sizeof(glm::mat4),
                                          cur_model_matrix_index * sizeof(glm::mat4),
                                          model_matrix_buffer);
                cur_model_matrix_index++;
            }
        }

        if(start_index != cur_model_matrix_index) {
            const auto& [vertex_buffer, index_buffer] = batch.mesh->get_buffers_for_frame(cur_frame_idx);
            // TODO: There's probably a better way to do this
            const std::vector<rhi::Buffer*> vertex_buffers = {7, vertex_buffer};
            cmds->bind_vertex_buffers(vertex_buffers);
            cmds->bind_index_buffer(index_buffer);

            cmds->draw_indexed_mesh(static_cast<uint32_t>(index_buffer->size / sizeof(uint32_t)), cur_model_matrix_index - start_index);
        }
    }

    RenderableId NovaRenderer::add_renderable_for_material(const FullMaterialPassName& material_name,
                                                           const StaticMeshRenderableData& renderable) {
        const RenderableId id = next_renderable_id.load();
        next_renderable_id.fetch_add(1);

        const auto pos = material_pass_keys.find(material_name);
        if(pos == material_pass_keys.end()) {
            NOVA_LOG(ERROR) << "No material named " << material_name.material_name << " for pass " << material_name.pass_name;
            return std::numeric_limits<uint64_t>::max();
        }

        // Figure out where to put the renderable
        const MaterialPassKey& pass_key = pos->second;

        Renderpass& renderpass = renderpasses.at(pass_key.renderpass_index);
        Pipeline& pipeline = renderpass.pipelines.at(pass_key.pipeline_index);
        MaterialPass& material = pipeline.passes.at(pass_key.material_pass_index);

        StaticMeshRenderCommand command = make_render_command(renderable, id);

        if(const auto itr = meshes.find(renderable.mesh); itr != meshes.end()) {
            const Mesh& mesh = itr->second;

            if(renderable.is_static) {
                bool need_to_add_batch = true;

                for(MeshBatch<StaticMeshRenderCommand>& batch : material.static_mesh_draws) {
                    if(batch.vertex_buffer == mesh.vertex_buffer) {
                        batch.commands.emplace_back(command);

                        need_to_add_batch = false;
                        break;
                    }
                }

                if(need_to_add_batch) {
                    MeshBatch<StaticMeshRenderCommand> batch;
                    batch.vertex_buffer = mesh.vertex_buffer;
                    batch.index_buffer = mesh.index_buffer;
                    batch.commands.emplace_back(command);

                    material.static_mesh_draws.emplace_back(batch);
                }
            }

        } else if(const auto proc_itr = proc_meshes.find(renderable.mesh); proc_itr != proc_meshes.end()) {
            if(renderable.is_static) {
                bool need_to_add_batch = false;

                for(ProceduralMeshBatch<StaticMeshRenderCommand>& batch : material.static_procedural_mesh_draws) {
                    if(batch.mesh.get_key() == renderable.mesh) {
                        batch.commands.emplace_back(command);

                        need_to_add_batch = false;
                        break;
                    }
                }

                if(need_to_add_batch) {
                    ProceduralMeshBatch<StaticMeshRenderCommand> batch(&proc_meshes, renderable.mesh);
                    batch.commands.emplace_back(command);

                    material.static_procedural_mesh_draws.emplace_back(batch);
                }
            }
        } else {
            NOVA_LOG(ERROR) << "Could not find a mesh with ID " << renderable.mesh;
        }

        return id;
    }

    rhi::RenderEngine* NovaRenderer::get_engine() const { return rhi.get(); }

    std::shared_ptr<NovaWindow> NovaRenderer::get_window() const { return window; }

    NovaRenderer* NovaRenderer::get_instance() { return instance.get(); }

    NovaRenderer* NovaRenderer::initialize(const NovaSettings& settings) {
        return (instance = std::make_unique<NovaRenderer>(settings)).get();
    }

    void NovaRenderer::deinitialize() { instance.reset(); }

    void NovaRenderer::create_global_allocator() {
        auto* heap = new uint8_t[global_memory_pool_size.b_count()];
        allocator_handle handle(new Mallocator);
        std::unique_ptr<AllocationStrategy> allocation_strategy = std::make_unique<BlockAllocationStrategy>(handle,
                                                                                                            global_memory_pool_size);

        global_allocator = std::make_shared<allocator_handle>(
            new SystemMemoryAllocator(heap, global_memory_pool_size, std::move(allocation_strategy)));
    }

    void NovaRenderer::create_global_gpu_pools() {
        const uint64_t mesh_memory_size = 512000000;
        ntl::Result<rhi::DeviceMemory*> memory_result = rhi->allocate_device_memory(mesh_memory_size,
                                                                                    rhi::MemoryUsage::DeviceOnly,
                                                                                    rhi::ObjectType::Buffer);
        const ntl::Result<DeviceMemoryResource*> mesh_memory_result = memory_result.map([&](rhi::DeviceMemory* memory) {
            auto* allocator = new BlockAllocationStrategy(*global_allocator.get(), Bytes(mesh_memory_size), 64_b);
            return new DeviceMemoryResource(memory, allocator);
        });

        if(mesh_memory_result) {
            mesh_memory = std::make_unique<DeviceMemoryResource>(*mesh_memory_result.value);

        } else {
            NOVA_LOG(ERROR) << "Could not create mesh memory pool: " << mesh_memory_result.error.to_string().c_str();
        }

        // Assume 65k things, plus we need space for the builtin ubos
        const uint64_t ubo_memory_size = sizeof(PerFrameUniforms) + sizeof(glm::mat4) * 0xFFFF;
        const ntl::Result<DeviceMemoryResource*>
            ubo_memory_result = rhi->allocate_device_memory(ubo_memory_size, rhi::MemoryUsage::DeviceOnly, rhi::ObjectType::Buffer)
                                    .map([=](rhi::DeviceMemory* memory) {
                                        auto* allocator = new BumpPointAllocationStrategy(Bytes(ubo_memory_size), Bytes(sizeof(glm::mat4)));
                                        return new DeviceMemoryResource(memory, allocator);
                                    });

        if(ubo_memory_result) {
            ubo_memory = std::make_unique<DeviceMemoryResource>(*ubo_memory_result.value);

        } else {
            NOVA_LOG(ERROR) << "Could not create mesh memory pool: " << ubo_memory_result.error.to_string().c_str();
        }

        // Staging buffers will be pooled, so we don't need a _ton_ of memory for them
        const Bytes staging_memory_size = 256_kb;
        const ntl::Result<DeviceMemoryResource*>
            staging_memory_result = rhi->allocate_device_memory(staging_memory_size.b_count(),
                                                                rhi::MemoryUsage::StagingBuffer,
                                                                rhi::ObjectType::Buffer)
                                        .map([=](rhi::DeviceMemory* memory) {
                                            auto* allocator = new BumpPointAllocationStrategy(staging_memory_size, 64_b);
                                            return new DeviceMemoryResource(memory, allocator);
                                        });

        if(staging_memory_result) {
            staging_buffer_memory = std::make_unique<DeviceMemoryResource>(*staging_memory_result.value);

        } else {
            NOVA_LOG(ERROR) << "Could not create staging buffer memory pool: " << staging_memory_result.error.to_string().c_str();
        }
    }

    void NovaRenderer::create_global_sync_objects() {
        const std::vector<rhi::Fence*>& fences = rhi->create_fences(NUM_IN_FLIGHT_FRAMES, true);
        for(uint32_t i = 0; i < NUM_IN_FLIGHT_FRAMES; i++) {
            frame_fences[i] = fences.at(i);
        }
    }

    void NovaRenderer::create_uniform_buffers() {
        // Buffer for per-frame uniform data
        rhi::BufferCreateInfo per_frame_data_create_info = {};
        per_frame_data_create_info.size = sizeof(PerFrameUniforms);
        per_frame_data_create_info.buffer_usage = rhi::BufferUsage::UniformBuffer;

        auto* per_frame_data_buffer = rhi->create_buffer(per_frame_data_create_info, *ubo_memory);
        builtin_buffers.emplace(PER_FRAME_DATA_NAME, per_frame_data_buffer);

        // Buffer for each drawcall's model matrix
        rhi::BufferCreateInfo model_matrix_buffer_create_info = {};
        model_matrix_buffer_create_info.size = sizeof(glm::mat4) * 0xFFFF;
        model_matrix_buffer_create_info.buffer_usage = rhi::BufferUsage::UniformBuffer;

        auto* model_matrix_buffer = rhi->create_buffer(model_matrix_buffer_create_info, *ubo_memory);
        builtin_buffers.emplace(MODEL_MATRIX_BUFFER_NAME, model_matrix_buffer);
    }
} // namespace nova::renderer
