#include "nova_renderer/nova_renderer.hpp"

#include <array>
#include <future>
#include <unordered_map>

#include <Tracy.hpp>
#include <TracyVulkan.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "nova_renderer/constants.hpp"
#include "nova_renderer/loading/renderpack_loading.hpp"
#include "nova_renderer/procedural_mesh.hpp"
#include "nova_renderer/rendergraph.hpp"
#include "nova_renderer/renderpack_data_conversions.hpp"
#include "nova_renderer/rhi/command_list.hpp"
#include "nova_renderer/rhi/swapchain.hpp"
#include "nova_renderer/ui_renderer.hpp"
#include "nova_renderer/util/platform.hpp"

#include "debugging/renderdoc.hpp"
#include "loading/renderpack/render_graph_builder.hpp"
#include "logging/console_log_stream.hpp"
#include "render_objects/uniform_structs.hpp"
#include "renderer/builtin/backbuffer_output_pass.hpp"

using namespace nova::mem;
using namespace operators;

namespace nova::renderer {
    static auto logger = spdlog::stdout_color_mt("NovaRenderer");

    struct BackbufferOutputPipelineCreateInfo : RhiGraphicsPipelineState {
        BackbufferOutputPipelineCreateInfo();
    };

    BackbufferOutputPipelineCreateInfo::BackbufferOutputPipelineCreateInfo() {
        name = BACKBUFFER_OUTPUT_PIPELINE_NAME;

        const std::string vertex_source{R"(
            struct VsInput {
                float2 position : POSITION;
            };

            struct VsOutput {
                float4 position : SV_POSITION;
                float2 uv : TEXCOORD;
            };

            VsOutput main(VsInput input) {
                VsOutput output;
                output.position = float4(input.position * 2.0 - 1.0, 0, 1);
                output.uv = input.position;

                return output;
            })"};
        const auto& vertex_spirv = renderpack::compile_shader(vertex_source, rhi::ShaderStage::Vertex, rhi::ShaderLanguage::Hlsl);
        if(vertex_spirv.empty()) {
            logger->error("Could not compile builtin backbuffer output vertex shader");
        }
        vertex_shader = {"/nova/shaders/backbuffer_output.vertex.hlsl", vertex_spirv};

        const std::string pixel_source{R"(
            [[vk::binding(0, 0)]]
            Texture2D ui_output : register(t0);

            [[vk::binding(1, 0)]]
            Texture2D scene_output : register(t1);

            [[vk::binding(2, 0)]]
            SamplerState tex_sampler : register(s0);

            struct VsOutput {
                float4 position : SV_POSITION;
                float2 uv : TEXCOORD;
            };

            float3 main(VsOutput input) : SV_Target {
                float4 ui_color = ui_output.Sample(tex_sampler, input.uv);
                float4 scene_color = scene_output.Sample(tex_sampler, input.uv);

                float3 combined_color = lerp(scene_color.rgb, ui_color.rgb, ui_color.a);

                return combined_color;
            })"};
        const auto& pixel_spirv = renderpack::compile_shader(pixel_source, rhi::ShaderStage::Pixel, rhi::ShaderLanguage::Hlsl);
        if(pixel_spirv.empty()) {
            logger->error("Could not compile builtin backbuffer output pixel shader");
        }
        pixel_shader = ShaderSource{.filename = "/nova/shaders/backbuffer_output.pixel.hlsl", .source = pixel_spirv};

        vertex_fields.emplace_back("position", rhi::VertexFieldFormat::Float2);

        // TODO: Figure out how to make the input textures into input attachments
        color_attachments.emplace_back(BACKBUFFER_NAME, rhi::PixelFormat::Rgba8, false);
    }

    static BackbufferOutputPipelineCreateInfo backbuffer_output_pipeline_create_info{};

    bool FullMaterialPassName::operator==(const FullMaterialPassName& other) const {
        return material_name == other.material_name && pass_name == other.pass_name;
    }

    NovaRenderer::NovaRenderer(const NovaSettings& settings) : settings{settings} {
        spdlog::flush_on(spdlog::level::err);

        ZoneScoped;
        create_global_allocators();

        initialize_virtual_filesystem();

        window = std::make_unique<NovaWindow>(settings);

        if(settings.debug.renderdoc.enabled) {
            ZoneScoped;
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

                    logger->info("Loaded RenderDoc successfully");

                    return 0;
                })
                .on_error([](const ntl::NovaError& error) { logger->error("%s", error.to_string()); });
        }

        {
            ZoneScoped;
            device = rhi::create_render_device(this->settings, *window);
        }

        swapchain = device->get_swapchain();

        create_global_sync_objects();

        create_global_samplers();

        create_resource_storage();

        create_builtin_render_targets();

        create_builtin_uniform_buffers();

        create_builtin_meshes();

        create_renderpass_manager();

        create_builtin_renderpasses();

        cameras.reserve(MAX_NUM_CAMERAS);
        camera_data = std::make_unique<PerFrameDeviceArray<CameraUboData>>(MAX_NUM_CAMERAS, settings.max_in_flight_frames, *device);
    }

    NovaRenderer::~NovaRenderer() {}

    NovaSettingsAccessManager& NovaRenderer::get_settings() { return settings; }

    void NovaRenderer::execute_frame() {
        {
            ZoneScoped;
            frame_count++;

            cur_frame_idx = device->get_swapchain()->acquire_next_swapchain_image();

            std::vector<rhi::RhiFence*> cur_frame_fences{frame_fences[cur_frame_idx]};

            device->wait_for_fences(cur_frame_fences);
            device->reset_fences(cur_frame_fences);

            FrameContext ctx = {};
            ctx.frame_count = frame_count;
            ctx.frame_idx = cur_frame_idx;
            ctx.nova = this;
            ctx.swapchain_framebuffer = swapchain->get_framebuffer(cur_frame_idx);
            ctx.swapchain_image = swapchain->get_image(cur_frame_idx);
            ctx.camera_matrix_buffer = camera_data->get_buffer_for_frame(cur_frame_idx);
            ctx.material_buffer = material_device_buffers[cur_frame_idx];

            rhi::RhiRenderCommandList* cmds = device->create_command_list(0,
                                                                          rhi::QueueType::Graphics,
                                                                          rhi::RhiRenderCommandList::Level::Primary);
            cmds->set_debug_name("RendergraphCommands");

            const auto images = get_all_images();

            cmds->bind_material_resources(ctx.camera_matrix_buffer,
                                          ctx.material_buffer->buffer,
                                          point_sampler,
                                          point_sampler,
                                          point_sampler,
                                          images);

            const auto& renderpass_order = rendergraph->calculate_renderpass_execution_order();

            for(const std::string& renderpass_name : renderpass_order) {
                auto* renderpass = rendergraph->get_renderpass(renderpass_name);
                renderpass->execute(*cmds, ctx);
            }

            // The rendergraph may update the camera and material data, so we upload the data at the end of the frame
            update_camera_matrix_buffer(cur_frame_idx);
            device->write_data_to_buffer(material_buffer->data(), ctx.material_buffer->size, ctx.material_buffer->buffer);

            device->submit_command_list(cmds, rhi::QueueType::Graphics, frame_fences[cur_frame_idx]);

            // Wait for the GPU to finish before presenting. This destroys pipelining and throughput, however at this time I'm not sure how
            // best to say "when GPU finishes this task, CPU should do something"
            device->wait_for_fences(cur_frame_fences);

            device->get_swapchain()->present(cur_frame_idx);
        }

        FrameMark;
#ifdef TRACY_ENABLE
        Tracyvk::NewFrame(renderer::RenderBackend::tracy_context);
        Tracyvk::Collect(renderer::RenderBackend::tracy_context);
#endif
    }

    void NovaRenderer::set_num_meshes(const uint32_t /* num_meshes */) { /* TODO? */
    }

    MeshId NovaRenderer::create_mesh(const MeshData& mesh_data) {
        if(mesh_data.num_vertex_attributes == 0) {
            logger->error("Can not add a mesh with zero vertex attributes");
        }

        if(mesh_data.num_indices == 0) {
            logger->error("Can not add a mesh with zero indices");
        }

        rhi::RhiBufferCreateInfo vertex_buffer_create_info;
        vertex_buffer_create_info.buffer_usage = rhi::BufferUsage::VertexBuffer;
        vertex_buffer_create_info.size = mesh_data.vertex_data_size;

        rhi::RhiBuffer* vertex_buffer = device->create_buffer(vertex_buffer_create_info);

        // TODO: Try to get staging buffers from a pool

        {
            rhi::RhiBufferCreateInfo staging_vertex_buffer_create_info = vertex_buffer_create_info;
            staging_vertex_buffer_create_info.buffer_usage = rhi::BufferUsage::StagingBuffer;

            rhi::RhiBuffer* staging_vertex_buffer = device->create_buffer(staging_vertex_buffer_create_info);
            device->write_data_to_buffer(mesh_data.vertex_data_ptr, vertex_buffer_create_info.size, staging_vertex_buffer);

            rhi::RhiRenderCommandList* vertex_upload_cmds = device->create_command_list(0,
                                                                                        rhi::QueueType::Transfer,
                                                                                        rhi::RhiRenderCommandList::Level::Primary);
            vertex_upload_cmds->set_debug_name("VertexDataUpload");
            vertex_upload_cmds->copy_buffer(vertex_buffer, 0, staging_vertex_buffer, 0, vertex_buffer_create_info.size);

            rhi::RhiResourceBarrier vertex_barrier = {};
            vertex_barrier.resource_to_barrier = vertex_buffer;
            vertex_barrier.old_state = rhi::ResourceState::CopyDestination;
            vertex_barrier.new_state = rhi::ResourceState::Common;
            vertex_barrier.access_before_barrier = rhi::ResourceAccess::CopyWrite;
            vertex_barrier.access_after_barrier = rhi::ResourceAccess::VertexAttributeRead;
            vertex_barrier.source_queue = rhi::QueueType::Transfer;
            vertex_barrier.destination_queue = rhi::QueueType::Graphics;
            vertex_barrier.buffer_memory_barrier.offset = 0;
            vertex_barrier.buffer_memory_barrier.size = vertex_buffer->size;

            vertex_upload_cmds->resource_barriers(rhi::PipelineStage::Transfer, rhi::PipelineStage::VertexInput, {vertex_barrier});

            device->submit_command_list(vertex_upload_cmds, rhi::QueueType::Transfer);

            // TODO: Barrier on the mesh's first usage
        }

        rhi::RhiBufferCreateInfo index_buffer_create_info;
        index_buffer_create_info.buffer_usage = rhi::BufferUsage::IndexBuffer;
        index_buffer_create_info.size = mesh_data.index_data_size;

        rhi::RhiBuffer* index_buffer = device->create_buffer(index_buffer_create_info);

        {
            rhi::RhiBufferCreateInfo staging_index_buffer_create_info = index_buffer_create_info;
            staging_index_buffer_create_info.buffer_usage = rhi::BufferUsage::StagingBuffer;
            rhi::RhiBuffer* staging_index_buffer = device->create_buffer(staging_index_buffer_create_info);
            device->write_data_to_buffer(mesh_data.index_data_ptr, index_buffer_create_info.size, staging_index_buffer);

            rhi::RhiRenderCommandList* indices_upload_cmds = device->create_command_list(0,
                                                                                         rhi::QueueType::Transfer,
                                                                                         rhi::RhiRenderCommandList::Level::Primary);
            indices_upload_cmds->set_debug_name("IndexDataUpload");
            indices_upload_cmds->copy_buffer(index_buffer, 0, staging_index_buffer, 0, index_buffer_create_info.size);

            rhi::RhiResourceBarrier index_barrier = {};
            index_barrier.resource_to_barrier = index_buffer;
            index_barrier.old_state = rhi::ResourceState::CopyDestination;
            index_barrier.new_state = rhi::ResourceState::Common;
            index_barrier.access_before_barrier = rhi::ResourceAccess::CopyWrite;
            index_barrier.access_after_barrier = rhi::ResourceAccess::IndexRead;
            index_barrier.source_queue = rhi::QueueType::Transfer;
            index_barrier.destination_queue = rhi::QueueType::Graphics;
            index_barrier.buffer_memory_barrier.offset = 0;
            index_barrier.buffer_memory_barrier.size = index_buffer->size;

            indices_upload_cmds->resource_barriers(rhi::PipelineStage::Transfer, rhi::PipelineStage::VertexInput, {index_barrier});

            device->submit_command_list(indices_upload_cmds, rhi::QueueType::Transfer);

            // TODO: Barrier on the mesh's first usage
        }

        // TODO: Clean up staging buffers

        Mesh mesh;
        mesh.num_vertex_attributes = mesh_data.num_vertex_attributes;
        mesh.vertex_buffer = vertex_buffer;
        mesh.index_buffer = index_buffer;
        mesh.num_indices = mesh_data.num_indices;

        const MeshId new_mesh_id = next_mesh_id;
        next_mesh_id++;
        meshes.emplace(new_mesh_id, mesh);

        return new_mesh_id;
    }

    ProceduralMeshAccessor NovaRenderer::create_procedural_mesh(const uint64_t vertex_size, const uint64_t index_size) {
        const MeshId our_id = next_mesh_id;
        next_mesh_id++;

        proc_meshes.emplace(our_id, ProceduralMesh{vertex_size, index_size, settings->max_in_flight_frames, device.get()});

        return ProceduralMeshAccessor{&proc_meshes, our_id};
    }

    std::optional<Mesh> NovaRenderer::get_mesh(const MeshId mesh_id) {
        if(const auto mesh_itr = meshes.find(mesh_id); mesh_itr != meshes.end()) {
            return mesh_itr->second;

        } else {
            return std::nullopt;
        }
    }

    void NovaRenderer::load_renderpack(const std::string& renderpack_name) {
        ZoneScoped;
        const renderpack::RenderpackData data = renderpack::load_renderpack_data(renderpack_name);

        if(renderpacks_loaded) {
            destroy_dynamic_resources();

            destroy_renderpasses();
            logger->debug("Resources from old renderpack destroyed");
        }

        create_dynamic_textures(data.resources.render_targets);
        logger->debug("Dynamic textures created");

        create_render_passes(data.graph_data.passes, data.pipelines);

        logger->debug("Created render passes");

        create_pipelines_and_materials(data.pipelines, data.materials);

        logger->debug("Created pipelines and materials");

        renderpacks_loaded = true;

        logger->debug("Renderpack %s loaded successfully", renderpack_name);
    }

    const std::vector<MaterialPass>& NovaRenderer::get_material_passes_for_pipeline(const std::string& pipeline) {
        return passes_by_pipeline.find(pipeline)->second;
    }

    std::optional<RenderpassMetadata> NovaRenderer::get_renderpass_metadata(const std::string& renderpass_name) const {
        return rendergraph->get_metadata_for_renderpass(renderpass_name);
    }

    void NovaRenderer::create_dynamic_textures(const std::vector<renderpack::TextureCreateInfo>& texture_create_infos) {
        ZoneScoped;
        for(const renderpack::TextureCreateInfo& create_info : texture_create_infos) {
            ZoneScoped;
            const auto size = create_info.format.get_size_in_pixels(device->get_swapchain()->get_size());

            const auto render_target = device_resources->create_render_target(create_info.name,
                                                                              size.x,
                                                                              size.y,
                                                                              create_info.format.pixel_format);

            dynamic_texture_infos.emplace(create_info.name, create_info);
        }
    }

    void NovaRenderer::create_render_passes(const std::vector<renderpack::RenderPassCreateInfo>& pass_create_infos,
                                            const std::vector<renderpack::PipelineData>& pipelines) const {
        ZoneScoped;
        device->set_num_renderpasses(static_cast<uint32_t>(pass_create_infos.size()));

        for(const renderpack::RenderPassCreateInfo& create_info : pass_create_infos) {
            ZoneScoped;
            auto* renderpass = new Renderpass(create_info.name);
            if(rendergraph->add_renderpass(renderpass, create_info, *device_resources) != nullptr) {
                for(const renderpack::PipelineData& pipeline : pipelines) {
                    if(pipeline.pass == create_info.name) {
                        renderpass->pipeline_names.emplace_back(pipeline.name);
                    }
                }
            } else {
                logger->error("Could not create renderpass %s", create_info.name);
            }
        }
    }

    void NovaRenderer::create_pipelines_and_materials(const std::vector<renderpack::PipelineData>& pipeline_create_infos,
                                                      const std::vector<renderpack::MaterialData>& materials) {
        ZoneScoped;
        for(const renderpack::PipelineData& rp_pipeline_state : pipeline_create_infos) {
            ZoneScoped;
            const auto pipeline_state = to_pipeline_state_create_info(rp_pipeline_state, *rendergraph);
            if(!pipeline_state) {
                logger->error("Could not create pipeline %s", rp_pipeline_state.name);
                continue;
            }

            // TODO: A way for renderpack pipelines to say if they're global or surface pipelines
            Pipeline pipeline;
            pipeline.pipeline = device->create_surface_pipeline(*pipeline_state);

            create_materials_for_pipeline(pipeline, materials, rp_pipeline_state.name);

            pipelines.emplace(rp_pipeline_state.name, std::move(pipeline));

            return;
        }
    }

    void NovaRenderer::create_materials_for_pipeline(const Pipeline& pipeline,
                                                     const std::vector<renderpack::MaterialData>& materials,
                                                     const std::string& pipeline_name) {
        ZoneScoped; // Determine the pipeline layout so the material can create descriptors for the pipeline

        MaterialPassKey template_key = {};
        template_key.pipeline_name = pipeline_name;

        // Large overestimate, but that's fine
        std::vector<MaterialPass> passes;
        passes.reserve(materials.size());

        for(const renderpack::MaterialData& material_data : materials) {
            for(const renderpack::MaterialPass& pass_data : material_data.passes) {
                if(pass_data.pipeline == pipeline_name) {
                    const auto& event_name = fmt::format("{}.{}", material_data.name, pass_data.name);
                    ZoneScoped;
                    MaterialPass pass = {};
                    pass.pipeline_interface = pipeline.pipeline_interface;

                    const FullMaterialPassName full_pass_name{pass_data.material_name, pass_data.name};

                    pass.name = full_pass_name;

                    MaterialPassMetadata pass_metadata{};
                    pass_metadata.data = pass_data;
                    material_metadatas.emplace(full_pass_name, pass_metadata);

                    MaterialPassKey key = template_key;
                    key.material_pass_index = static_cast<uint32_t>(passes.size());

                    material_pass_keys.emplace(full_pass_name, key);

                    passes.push_back(pass);
                }
            }
        }

        passes_by_pipeline.emplace(pipeline.pipeline->name, passes);
    }

    void NovaRenderer::update_camera_matrix_buffer(const uint32_t frame_idx) {
        ZoneScoped;
        for(const Camera& cam : cameras) {
            if(cam.is_active) {
                auto& data = camera_data->at(cam.index);
                data.previous_view = data.view;
                data.previous_projection = data.projection;

                data.view = translate({}, cam.position);
                data.view = rotate(data.view, cam.rotation.x, {1, 0, 0});
                data.view = rotate(data.view, cam.rotation.y, {0, 1, 0});
                data.view = rotate(data.view, cam.rotation.z, {0, 0, 1});

                if(cam.field_of_view > 0) {
                    data.projection = glm::perspective(cam.field_of_view, cam.aspect_ratio, cam.near_plane, cam.far_plane);

                } else {
                    const auto framebuffer_size = device->get_swapchain()->get_size();
                    glm::mat4 ui_matrix{
                        {2.0f, 0.0f, 0.0f, -1.0f},
                        {0.0f, 2.0f, 0.0f, -1.0f},
                        {0.0f, 0.0f, -1.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f, 1.0f},
                    };
                    ui_matrix[0][0] /= framebuffer_size.x;
                    ui_matrix[1][1] /= framebuffer_size.y;
                    data.projection = ui_matrix;
                }
            }
        }

        camera_data->upload_to_device(frame_idx);
    }

    std::vector<rhi::RhiImage*> NovaRenderer::get_all_images() {
        std::vector<rhi::RhiImage*> images{};

        const auto& textures = device_resources->get_all_textures();
        images.reserve(textures.size());

        for(const TextureResource& texture : textures) {
            images.emplace_back(texture.image);
        }

        return images;
    }

    void NovaRenderer::destroy_dynamic_resources() {
        ZoneScoped;
        if(loaded_renderpack) {
            for(const renderpack::TextureCreateInfo& tex_data : loaded_renderpack->resources.render_targets) {
                device_resources->destroy_render_target(tex_data.name);
            }

            logger->debug("Deleted all dynamic textures from renderpack %s", loaded_renderpack->name);
        }
    }

    void NovaRenderer::destroy_renderpasses() {
        ZoneScoped;
        for(const renderpack::RenderPassCreateInfo& renderpass : loaded_renderpack->graph_data.passes) {
            rendergraph->destroy_renderpass(renderpass.name);
        }
    }

    rhi::RhiSampler* NovaRenderer::get_point_sampler() const { return point_sampler; }

    Pipeline* NovaRenderer::find_pipeline(const std::string& pipeline_name) {
        if(const auto pipeline_itr = pipelines.find(pipeline_name); pipeline_itr != pipelines.end()) {
            return &pipeline_itr->second;
        }

        return nullptr;
    }

    RenderableId NovaRenderer::add_renderable_for_material(const FullMaterialPassName& material_name,
                                                           const StaticMeshRenderableCreateInfo& create_info) {
        ZoneScoped;
        const RenderableId id = next_renderable_id.load();
        next_renderable_id.fetch_add(1);

        const auto pass_key_itr = material_pass_keys.find(material_name);
        if(pass_key_itr == material_pass_keys.end()) {
            logger->error("No material named %s for pass %s", material_name.material_name, material_name.pass_name);
            return std::numeric_limits<uint64_t>::max();
        }

        const auto& pass_key = pass_key_itr->second;

        RenderableKey key;
        key.pipeline_name = pass_key.pipeline_name;
        key.material_pass_idx = pass_key.material_pass_index;

        // Figure out where to put the renderable
        auto materials_itr = passes_by_pipeline.find(pass_key.pipeline_name);

        if(materials_itr == passes_by_pipeline.end()) {
            return std::numeric_limits<uint64_t>::max();
        }

        auto& material = materials_itr->second[pass_key.material_pass_index];

        StaticMeshRenderCommand command = make_render_command(create_info, id);

        if(const auto& mesh_itr = meshes.find(create_info.mesh); mesh_itr != meshes.end()) {
            const auto& mesh = mesh_itr->second;
            if(create_info.is_static) {
                key.type = RenderableType::StaticMesh;
                bool need_to_add_batch = true;

                uint32_t batch_idx = 0;
                for(MeshBatch<StaticMeshRenderCommand>& batch : material.static_mesh_draws) {
                    if(batch.vertex_buffer == mesh.vertex_buffer) {
                        key.batch_idx = batch_idx;
                        key.renderable_idx = static_cast<uint32_t>(batch.commands.size());

                        batch.commands.emplace_back(command);

                        need_to_add_batch = false;
                        break;
                    }

                    batch_idx++;
                }

                if(need_to_add_batch) {
                    MeshBatch<StaticMeshRenderCommand> batch;
                    batch.num_vertex_attributes = mesh.num_vertex_attributes;
                    batch.num_indices = mesh.num_indices;
                    batch.vertex_buffer = mesh.vertex_buffer;
                    batch.index_buffer = mesh.index_buffer;
                    batch.commands.emplace_back(command);

                    key.batch_idx = static_cast<uint32_t>(material.static_mesh_draws.size());
                    key.renderable_idx = 0;

                    material.static_mesh_draws.emplace_back(batch);
                }
            }

        } else if(const auto& proc_mesh_itr = proc_meshes.find(create_info.mesh); proc_mesh_itr != proc_meshes.end()) {
            if(create_info.is_static) {
                key.type = RenderableType::ProceduralMesh;
                bool need_to_add_batch = false;

                uint32_t batch_idx = 0;
                for(ProceduralMeshBatch<StaticMeshRenderCommand>& batch : material.static_procedural_mesh_draws) {
                    if(batch.mesh.get_key() == create_info.mesh) {
                        key.batch_idx = batch_idx;
                        key.renderable_idx = static_cast<uint32_t>(batch.commands.size());

                        batch.commands.emplace_back(command);

                        need_to_add_batch = false;
                        break;
                    }

                    batch_idx++;
                }

                if(need_to_add_batch) {
                    ProceduralMeshBatch<StaticMeshRenderCommand> batch{&proc_meshes, create_info.mesh};
                    batch.commands.emplace_back(command);

                    key.batch_idx = static_cast<uint32_t>(material.static_mesh_draws.size());
                    key.renderable_idx = 0;

                    material.static_procedural_mesh_draws.emplace_back(batch);
                }
            }
        } else {
            logger->error("Could not find a mesh with ID %u", create_info.mesh);
            return std::numeric_limits<uint64_t>::max();
        }

        renderable_keys.emplace(id, key);

        return id;
    }

    void NovaRenderer::update_renderable(RenderableId renderable, const StaticMeshRenderableUpdateData& update_data) {
        ZoneScoped;
        const auto& key_itr = renderable_keys.find(renderable);
        if(key_itr == renderable_keys.end()) {
            logger->error("Could not update renderable %u", renderable);
            return;
        }

        auto passes_itr = passes_by_pipeline.find(key_itr->second.pipeline_name);
        if(passes_itr == passes_by_pipeline.end()) {
            logger->error("Could not find draws for pipeline %s, as requested by renderable %u", key_itr->second.pipeline_name, renderable);
            return;
        }

        const auto& key = key_itr->second;
        auto& material_pass = passes_itr->second[key.material_pass_idx];

        auto command = [&] {
            switch(key.type) {
                case RenderableType::StaticMesh: {
                    auto& batch = material_pass.static_mesh_draws[key.batch_idx];
                    return batch.commands[key.renderable_idx];
                }

                case RenderableType::ProceduralMesh: {
                    auto& batch = material_pass.static_procedural_mesh_draws[key.batch_idx];
                    return batch.commands[key.renderable_idx];
                }
            }
        }();

        command.is_visible = update_data.visible;

        // TODO: Make sure my matrix math is correct
        command.model_matrix = glm::translate(glm::mat4{1}, update_data.position);
        command.model_matrix = glm::scale(command.model_matrix, update_data.scale);
        command.model_matrix = glm::rotate(command.model_matrix, update_data.rotation.x, {1, 0, 0});
        command.model_matrix = glm::rotate(command.model_matrix, update_data.rotation.y, {0, 1, 0});
        command.model_matrix = glm::rotate(command.model_matrix, update_data.rotation.z, {0, 0, 1});
    }

    CameraAccessor NovaRenderer::create_camera(const CameraCreateInfo& create_info) {
        const auto idx = cameras.size();
        cameras.emplace_back(create_info);

        CameraAccessor camera{&cameras, idx};

        camera->index = camera_data->get_next_free_slot();

        return camera;
    }

    rhi::RenderDevice& NovaRenderer::get_device() const { return *device; }

    NovaWindow& NovaRenderer::get_window() const { return *window; }

    DeviceResources& NovaRenderer::get_resource_manager() const { return *device_resources; }

    void NovaRenderer::initialize_virtual_filesystem() {
        // The host application MUST register its data directory before initializing Nova

        const auto vfs = filesystem::VirtualFilesystem::get_instance();

        const auto renderpacks_directory = vfs->get_folder_accessor(RENDERPACK_DIRECTORY);

        vfs->add_resource_root(renderpacks_directory);
    }

    void NovaRenderer::create_global_sync_objects() { frame_fences = device->create_fences(settings->max_in_flight_frames, true); }

    void NovaRenderer::create_global_samplers() {
        {
            // Default sampler create info will give us a delicious point sampler
            point_sampler = device->create_sampler({});
        }
    }

    void NovaRenderer::create_resource_storage() { device_resources = std::make_unique<DeviceResources>(*this); }

    void NovaRenderer::create_builtin_render_targets() {
        const auto& swapchain_size = device->get_swapchain()->get_size();

        {
            const auto scene_output = device_resources->create_render_target(SCENE_OUTPUT_RT_NAME,
                                                                             swapchain_size.x,
                                                                             swapchain_size.y,
                                                                             rhi::PixelFormat::Rgba8,
                                                                             true);

            if(!scene_output) {
                logger->error("Could not create scene output render target");

            } else {
                dynamic_texture_infos
                    .emplace(SCENE_OUTPUT_RT_NAME,
                             renderpack::TextureCreateInfo{.name = SCENE_OUTPUT_RT_NAME,
                                                           .usage = renderpack::ImageUsage::RenderTarget,
                                                           .format = {.pixel_format = rhi::PixelFormat::Rgba8,
                                                                      .dimension_type = renderpack::TextureDimensionType::ScreenRelative,
                                                                      .width = 1,
                                                                      .height = 1}});
            }
        }

        {
            const auto ui_output = device_resources->create_render_target(UI_OUTPUT_RT_NAME,
                                                                          swapchain_size.x,
                                                                          swapchain_size.y,
                                                                          rhi::PixelFormat::Rgba8,
                                                                          true);

            if(!ui_output) {
                logger->error("Could not create UI output render target");

            } else {
                dynamic_texture_infos
                    .emplace(UI_OUTPUT_RT_NAME,
                             renderpack::TextureCreateInfo{.name = UI_OUTPUT_RT_NAME,
                                                           .usage = renderpack::ImageUsage::RenderTarget,
                                                           .format = {.pixel_format = rhi::PixelFormat::Rgba8,
                                                                      .dimension_type = renderpack::TextureDimensionType::ScreenRelative,
                                                                      .width = 1,
                                                                      .height = 1}});
            }
        }
    }

    void NovaRenderer::create_builtin_uniform_buffers() {
        material_buffer = std::make_unique<MaterialDataBuffer>();
        for(uint32_t i = 0; i < settings->max_in_flight_frames; i++) {
            const auto buffer_name = fmt::format("{}_{}", MATERIAL_DATA_BUFFER_NAME, i);
            if(auto buffer = device_resources->create_uniform_buffer(buffer_name, MATERIAL_BUFFER_SIZE); buffer) {
                builtin_buffer_names.emplace_back(buffer_name);
                material_device_buffers.emplace_back(*buffer);

            } else {
                logger->error("Could not create builtin buffer %s", buffer_name);
            }
        }

        if(device_resources->create_uniform_buffer(PER_FRAME_DATA_NAME, sizeof(PerFrameUniforms))) {
            builtin_buffer_names.emplace_back(PER_FRAME_DATA_NAME);

        } else {
            logger->error("Could not create builtin buffer %s", PER_FRAME_DATA_NAME);
        }

        if(device_resources->create_uniform_buffer(MODEL_MATRIX_BUFFER_NAME, sizeof(glm::mat4) * 0xFFFF)) {
            builtin_buffer_names.emplace_back(MODEL_MATRIX_BUFFER_NAME);

        } else {
            logger->error("Could not create builtin buffer %s", MODEL_MATRIX_BUFFER_NAME);
        }
    }

    void NovaRenderer::create_builtin_meshes() {
        const static std::array TRIANGLE_VERTICES{0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 2.0f};
        const static std::array TRIANGLE_INDICES{0, 1, 2};

        const MeshData fullscreen_triangle_data{1,
                                                3,
                                                TRIANGLE_VERTICES.data(),
                                                TRIANGLE_VERTICES.size() * sizeof(float),
                                                TRIANGLE_INDICES.data(),
                                                TRIANGLE_INDICES.size() * sizeof(uint32_t)};

        fullscreen_triangle_id = create_mesh(fullscreen_triangle_data);
    }

    void NovaRenderer::create_renderpass_manager() { rendergraph = std::make_unique<Rendergraph>(*device); }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void NovaRenderer::create_builtin_renderpasses() {
        const auto& ui_output = *device_resources->get_render_target(UI_OUTPUT_RT_NAME);
        const auto& scene_output = *device_resources->get_render_target(SCENE_OUTPUT_RT_NAME);

        backbuffer_output_pipeline_create_info.viewport_size = device->get_swapchain()->get_size();
        auto backbuffer_pipeline = device->create_global_pipeline(*backbuffer_output_pipeline_create_info);
        if(rendergraph->create_renderpass<BackbufferOutputRenderpass>(*device_resources,
                                                                      ui_output->image,
                                                                      scene_output->image,
                                                                      point_sampler,
                                                                      std::move(backbuffer_pipeline),
                                                                      fullscreen_triangle_id,
                                                                      *device) == nullptr) {

            logger->error("Could not create the backbuffer output renderpass");
        }
    }

    void NovaRenderer::create_builtin_pipelines() {}
} // namespace nova::renderer
