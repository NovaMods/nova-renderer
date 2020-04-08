#include "nova_renderer/nova_renderer.hpp"

#pragma warning(push, 0)
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <minitrace.h>
#include <spirv_glsl.hpp>
#pragma warning(pop)

#include <rx/core/array.h>
#include <rx/core/global.h>
#include <rx/core/hash.h>
#include <rx/core/log.h>
#include <rx/core/map.h>
#include <rx/core/memory/bump_point_allocator.h>

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

static rx::global_group g_nova_globals{"Nova"};

RX_LOG("nova", logger);

static rx::global<nova::StdoutStream> stdout_stream{"system", "stdout_stream"};

void init_rex() {
    static bool initialized = false;

    if(!initialized) {
        rx::globals::link();

        rx::global_group* system_group{rx::globals::find("system")};

        // Explicitly initialize globals that need to be initialized in a specific
        // order for things to work.
        system_group->find("allocator")->init();
        stdout_stream.init();
        system_group->find("logger")->init();

        const auto subscribed = rx::log::subscribe(&stdout_stream);
        if(!subscribed) {
            fprintf(stderr, "Could not subscribe to logger");
        }

        rx::globals::init();

        initialized = true;
    }
}

void rex_fini() {
    static bool deinitialized = false;

    if(!deinitialized) {
        rx::global_group* system_group{rx::globals::find("system")};

        rx::globals::fini();

        system_group->find("logger")->fini();
        stdout_stream.fini();
        system_group->find("allocator")->fini();

        deinitialized = true;
    }
}

namespace nova::renderer {
    struct RX_HINT_EMPTY_BASES BackbufferOutputPipelineCreateInfo : RhiGraphicsPipelineState {
        BackbufferOutputPipelineCreateInfo();
    };

    BackbufferOutputPipelineCreateInfo::BackbufferOutputPipelineCreateInfo() {
        name = BACKBUFFER_OUTPUT_PIPELINE_NAME;

        const rx::string vertex_source{R"(
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
        if(vertex_spirv.is_empty()) {
            logger->error("Could not compile builtin backbuffer output vertex shader");
        }
        vertex_shader = {"/nova/shaders/backbuffer_output.vertex.hlsl", vertex_spirv};

        const rx::string pixel_source{R"(
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
        if(pixel_spirv.is_empty()) {
            logger->error("Could not compile builtin backbuffer output pixel shader");
        }
        pixel_shader = {"/nova/shaders/backbuffer_output.pixel.hlsl", pixel_spirv};

        vertex_fields.emplace_back("position", rhi::VertexFieldFormat::Float2);

        // TODO: Figure out how to make the input textures into input attachments
        color_attachments.emplace_back(BACKBUFFER_NAME, rhi::PixelFormat::Rgba8, false);
    }

    static rx::global<BackbufferOutputPipelineCreateInfo> backbuffer_output_pipeline_create_info{"Nova",
                                                                                                 "BackbufferOutputPipelineCreateInfo"};

    bool FullMaterialPassName::operator==(const FullMaterialPassName& other) const {
        return material_name == other.material_name && pass_name == other.pass_name;
    }

    rx_size FullMaterialPassName::hash() const {
        const rx_size material_name_hash = material_name.hash();
        const rx_size pass_name_hash = pass_name.hash();

        return rx::hash_combine(material_name_hash, pass_name_hash);
    }

    NovaRenderer::NovaRenderer(const NovaSettings& settings_in)
        : settings{settings_in}, global_allocator{&rx::memory::system_allocator::instance()}, cameras{*global_allocator} {
        mtr_init("trace.json");

        MTR_META_PROCESS_NAME("NovaRenderer");
        MTR_META_THREAD_NAME("Main");

        MTR_SCOPE("Init", "nova_renderer::nova_renderer");

        create_global_allocators();

        initialize_virtual_filesystem();

        window = rx::make_ptr<NovaWindow>(*global_allocator, settings_in);

        if(settings_in.debug.renderdoc.enabled) {
            MTR_SCOPE("Init", "LoadRenderdoc");
            auto rd_load_result = load_renderdoc(settings_in.debug.renderdoc.renderdoc_dll_path);

            rd_load_result
                .map([&](RENDERDOC_API_1_3_0* api) {
                    render_doc = api;

                    render_doc->SetCaptureFilePathTemplate(settings_in.debug.renderdoc.capture_path);

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
            MTR_SCOPE("Init", "InitRenderDevice");
            device = rhi::create_render_device(this->settings, *window, *global_allocator);
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
        camera_data = rx::make_ptr<PerFrameDeviceArray<CameraUboData>>(*global_allocator,
                                                                       MAX_NUM_CAMERAS,
                                                                       settings_in.max_in_flight_frames,
                                                                       *device,
                                                                       *global_allocator);

        initialize_material_resource_binder();
    }

    NovaRenderer::~NovaRenderer() {
        mtr_flush();
        mtr_shutdown();
    }

    NovaSettingsAccessManager& NovaRenderer::get_settings() { return settings; }

    rx::memory::allocator& NovaRenderer::get_global_allocator() const { return *global_allocator; }

    void NovaRenderer::execute_frame() {
        {
            MTR_SCOPE("NovaRenderer", "execute_frame");
            frame_count++;

            rx::memory::bump_point_allocator& frame_allocator = *frame_allocators[frame_count % settings->max_in_flight_frames];
            frame_allocator.reset();

            cur_frame_idx = device->get_swapchain()->acquire_next_swapchain_image(frame_allocator);

            const rx::vector<rhi::RhiFence*> cur_frame_fences{*global_allocator, rx::array{frame_fences[cur_frame_idx].get()}};

            device->wait_for_fences(cur_frame_fences);
            device->reset_fences(cur_frame_fences);

            FrameContext ctx = {};
            ctx.frame_count = frame_count;
            ctx.frame_idx = cur_frame_idx;
            ctx.nova = this;
            ctx.allocator = &frame_allocator;
            ctx.swapchain_framebuffer = swapchain->get_framebuffer(cur_frame_idx);
            ctx.swapchain_image = swapchain->get_image(cur_frame_idx);
            ctx.camera_matrix_buffer = camera_data->get_buffer_for_frame(cur_frame_idx);
            ctx.material_buffer = material_device_buffers[cur_frame_idx];

            rx::ptr<rhi::RhiRenderCommandList> cmds = device->create_command_list(0,
                                                                                  rhi::QueueType::Graphics,
                                                                                  rhi::RhiRenderCommandList::Level::Primary,
                                                                                  frame_allocator);
            cmds->set_debug_name("RendergraphCommands");

            material_resource_binder->bind_buffer("material_buffer", ctx.material_buffer->buffer.get());
            material_resource_binder->bind_buffer("cameras", ctx.camera_matrix_buffer);
            const auto images = get_all_images(frame_allocator);
            material_resource_binder->bind_image_array("textures", images);

            cmds->bind_resources(*material_resource_binder);

            const auto& renderpass_order = rendergraph->calculate_renderpass_execution_order();

            renderpass_order.each_fwd([&](const rx::string& renderpass_name) {
                auto* renderpass = rendergraph->get_renderpass(renderpass_name);
                renderpass->execute(*cmds, ctx);
            });

            // The rendergraph may update the camera and material data, so we upload the data at the end of the frame
            update_camera_matrix_buffer(cur_frame_idx);
            device->write_data_to_buffer(material_buffer->data(), ctx.material_buffer->size, *ctx.material_buffer->buffer);

            cmds->set_checkpoint("frame finished");

            device->submit_command_list(rx::utility::move(cmds), rhi::QueueType::Graphics, *frame_fences[cur_frame_idx]);

            // Wait for the GPU to finish before presenting. This destroys pipelining and throughput, however at this time I'm not sure how
            // best to say "when GPU finishes this task, CPU should do something"
            // device->wait_for_fences(cur_frame_fences);

            device->get_swapchain()->present(cur_frame_idx);

            {
                MTR_SCOPE("NovaRenderer", "Flush Log");
                rx::log::flush();
            }
        }
        mtr_flush();
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

        rx::ptr<rhi::RhiBuffer> vertex_buffer = device->create_buffer(vertex_buffer_create_info, *global_allocator);

        // TODO: Try to get staging buffers from a pool

        {
            rhi::RhiBufferCreateInfo staging_vertex_buffer_create_info = vertex_buffer_create_info;
            staging_vertex_buffer_create_info.buffer_usage = rhi::BufferUsage::StagingBuffer;

            rx::ptr<rhi::RhiBuffer> staging_vertex_buffer = device->create_buffer(staging_vertex_buffer_create_info, *global_allocator);
            device->write_data_to_buffer(mesh_data.vertex_data_ptr, vertex_buffer_create_info.size, *staging_vertex_buffer);

            rx::ptr<rhi::RhiRenderCommandList> vertex_upload_cmds = device->create_command_list(0,
                                                                                                rhi::QueueType::Graphics,
                                                                                                rhi::RhiRenderCommandList::Level::Primary,
                                                                                                *global_allocator);
            vertex_upload_cmds->set_debug_name("VertexDataUpload");
            vertex_upload_cmds->copy_buffer(*vertex_buffer, 0, *staging_vertex_buffer, 0, vertex_buffer_create_info.size);

            rhi::RhiResourceBarrier vertex_barrier = {};
            vertex_barrier.resource_to_barrier = vertex_buffer.get();
            vertex_barrier.old_state = rhi::ResourceState::CopyDestination;
            vertex_barrier.new_state = rhi::ResourceState::Common;
            vertex_barrier.access_before_barrier = rhi::ResourceAccess::CopyWrite;
            vertex_barrier.access_after_barrier = rhi::ResourceAccess::VertexAttributeRead;
            vertex_barrier.source_queue = rhi::QueueType::Graphics;
            vertex_barrier.destination_queue = rhi::QueueType::Graphics;
            vertex_barrier.buffer_memory_barrier.offset = 0;
            vertex_barrier.buffer_memory_barrier.size = vertex_buffer->size;

            rx::vector<rhi::RhiResourceBarrier> barriers{*global_allocator};
            barriers.push_back(vertex_barrier);
            vertex_upload_cmds->resource_barriers(rhi::PipelineStage::Transfer, rhi::PipelineStage::VertexInput, barriers);

            device->submit_command_list(rx::utility::move(vertex_upload_cmds), rhi::QueueType::Graphics);
        }

        rhi::RhiBufferCreateInfo index_buffer_create_info;
        index_buffer_create_info.buffer_usage = rhi::BufferUsage::IndexBuffer;
        index_buffer_create_info.size = mesh_data.index_data_size;

        rx::ptr<rhi::RhiBuffer> index_buffer = device->create_buffer(index_buffer_create_info, *global_allocator);

        {
            rhi::RhiBufferCreateInfo staging_index_buffer_create_info = index_buffer_create_info;
            staging_index_buffer_create_info.buffer_usage = rhi::BufferUsage::StagingBuffer;
            rx::ptr<rhi::RhiBuffer> staging_index_buffer = device->create_buffer(staging_index_buffer_create_info, *global_allocator);
            device->write_data_to_buffer(mesh_data.index_data_ptr, index_buffer_create_info.size, *staging_index_buffer);

            rx::ptr<rhi::RhiRenderCommandList> indices_upload_cmds = device->create_command_list(0,
                                                                                                 rhi::QueueType::Graphics,
                                                                                                 rhi::RhiRenderCommandList::Level::Primary,
                                                                                                 *global_allocator);
            indices_upload_cmds->set_debug_name("IndexDataUpload");
            indices_upload_cmds->copy_buffer(*index_buffer, 0, *staging_index_buffer, 0, index_buffer_create_info.size);

            rhi::RhiResourceBarrier index_barrier = {};
            index_barrier.resource_to_barrier = index_buffer.get();
            index_barrier.old_state = rhi::ResourceState::CopyDestination;
            index_barrier.new_state = rhi::ResourceState::Common;
            index_barrier.access_before_barrier = rhi::ResourceAccess::CopyWrite;
            index_barrier.access_after_barrier = rhi::ResourceAccess::IndexRead;
            index_barrier.source_queue = rhi::QueueType::Graphics;
            index_barrier.destination_queue = rhi::QueueType::Graphics;
            index_barrier.buffer_memory_barrier.offset = 0;
            index_barrier.buffer_memory_barrier.size = index_buffer->size;

            rx::vector<rhi::RhiResourceBarrier> barriers{*global_allocator};
            barriers.push_back(index_barrier);
            indices_upload_cmds->resource_barriers(rhi::PipelineStage::Transfer, rhi::PipelineStage::VertexInput, barriers);

            device->submit_command_list(rx::utility::move(indices_upload_cmds), rhi::QueueType::Graphics);
        }

        // TODO: Clean up staging buffers

        Mesh mesh;
        mesh.num_vertex_attributes = mesh_data.num_vertex_attributes;
        mesh.vertex_buffer = rx::utility::move(vertex_buffer);
        mesh.index_buffer = rx::utility::move(index_buffer);
        mesh.num_indices = mesh_data.num_indices;

        const MeshId new_mesh_id = next_mesh_id;
        next_mesh_id++;
        meshes.insert(new_mesh_id, rx::utility::move(mesh));

        return new_mesh_id;
    }

    ProceduralMeshAccessor NovaRenderer::create_procedural_mesh(const uint64_t vertex_size, const uint64_t index_size) {
        const MeshId our_id = next_mesh_id;
        next_mesh_id++;

        proc_meshes.insert(our_id, ProceduralMesh{vertex_size, index_size, settings->max_in_flight_frames, device.get()});

        return ProceduralMeshAccessor{&proc_meshes, our_id};
    }

    rx::optional<Mesh*> NovaRenderer::get_mesh(const MeshId mesh_id) {
        if(auto* mesh = meshes.find(mesh_id)) {
            return mesh;

        } else {
            return rx::nullopt;
        }
    }

    void NovaRenderer::load_renderpack(const rx::string& renderpack_name) {
        MTR_SCOPE("RenderpackLoading", "load_renderpack");

        const renderpack::RenderpackData data = renderpack::load_renderpack_data(renderpack_name);

        if(renderpacks_loaded) {
            destroy_dynamic_resources();

            destroy_renderpasses();
            logger->verbose("Resources from old renderpack destroyed");
        }

        create_dynamic_textures(data.resources.render_targets);
        logger->verbose("Dynamic textures created");

        create_render_passes(data.graph_data.passes, data.pipelines);

        logger->verbose("Created render passes");

        create_pipelines_and_materials(data.pipelines, data.materials);

        logger->verbose("Created pipelines and materials");

        renderpacks_loaded = true;

        logger->verbose("Renderpack %s loaded successfully", renderpack_name);
    }

    const rx::vector<MaterialPass>& NovaRenderer::get_material_passes_for_pipeline(const rx::string& pipeline) {
        return *passes_by_pipeline.find(pipeline);
    }

    rx::optional<RenderpassMetadata> NovaRenderer::get_renderpass_metadata(const rx::string& renderpass_name) const {
        return rendergraph->get_metadata_for_renderpass(renderpass_name);
    }

    void NovaRenderer::create_dynamic_textures(const rx::vector<renderpack::TextureCreateInfo>& texture_create_infos) {
        MTR_SCOPE("create_dynamic_textures", "Self");
        texture_create_infos.each_fwd([&](const renderpack::TextureCreateInfo& create_info) {
            MTR_SCOPE("create_dynamic_textures", create_info.name.data());
            const auto size = create_info.format.get_size_in_pixels(device->get_swapchain()->get_size());

            const auto render_target = device_resources->create_render_target(create_info.name,
                                                                              size.x,
                                                                              size.y,
                                                                              create_info.format.pixel_format,
                                                                              *renderpack_allocator);

            dynamic_texture_infos.insert(create_info.name, create_info);
        });
    }

    void NovaRenderer::create_render_passes(const rx::vector<renderpack::RenderPassCreateInfo>& pass_create_infos,
                                            const rx::vector<renderpack::PipelineData>& new_pipelines) const {
        MTR_SCOPE("create_render_passes", "Self");
        device->set_num_renderpasses(static_cast<uint32_t>(pass_create_infos.size()));

        pass_create_infos.each_fwd([&](const renderpack::RenderPassCreateInfo& create_info) {
            MTR_SCOPE("create_render_passes", create_info.name.data());
            auto* renderpass = global_allocator->create<Renderpass>(create_info.name);
            if(rendergraph->add_renderpass(renderpass, create_info, *device_resources) != nullptr) {
                new_pipelines.each_fwd([&](const renderpack::PipelineData& pipeline) {
                    if(pipeline.pass == create_info.name) {
                        renderpass->pipeline_names.emplace_back(pipeline.name);
                    }
                });
            } else {
                logger->error("Could not create renderpass %s", create_info.name);
            }
        });
    }

    void NovaRenderer::create_pipelines_and_materials(const rx::vector<renderpack::PipelineData>& pipeline_create_infos,
                                                      const rx::vector<renderpack::MaterialData>& materials) {
        MTR_SCOPE("create_pipelines_and_materials", "Self");
        pipeline_create_infos.each_fwd([&](const renderpack::PipelineData& rp_pipeline_state) {
            MTR_SCOPE("create_pipelines_and_materials", rp_pipeline_state.name.data());
            const auto pipeline_state = to_pipeline_state_create_info(rp_pipeline_state, *rendergraph);
            if(!pipeline_state) {
                logger->error("Could not create pipeline %s", rp_pipeline_state.name);
                return false;
            }

            // TODO: A way for renderpack pipelines to say if they're global or surface pipelines
            Pipeline pipeline;
            pipeline.pipeline = device->create_surface_pipeline(*pipeline_state, *renderpack_allocator);

            create_materials_for_pipeline(pipeline, materials, rp_pipeline_state.name);

            pipelines.insert(rp_pipeline_state.name, rx::utility::move(pipeline));

            return true;
        });
    }

    void NovaRenderer::create_materials_for_pipeline(const Pipeline& pipeline,
                                                     const rx::vector<renderpack::MaterialData>& materials,
                                                     const rx::string& pipeline_name) {
        MTR_SCOPE("create_materials_for_pipeline", "Self");
        // Determine the pipeline layout so the material can create descriptors for the pipeline

        MaterialPassKey template_key = {};
        template_key.pipeline_name = pipeline_name;

        // Large overestimate, but that's fine
        rx::vector<MaterialPass> passes;
        passes.reserve(materials.size());

        materials.each_fwd([&](const renderpack::MaterialData& material_data) {
            material_data.passes.each_fwd([&](const renderpack::MaterialPass& pass_data) {
                if(pass_data.pipeline == pipeline_name) {
                    const auto& event_name = rx::string::format("%s.%s", material_data.name, pass_data.name);
                    MTR_SCOPE("create_materials_for_pipeline", event_name.data());
                    MaterialPass pass = {};
                    pass.pipeline_interface = pipeline.pipeline_interface;

                    const FullMaterialPassName full_pass_name{pass_data.material_name, pass_data.name};

                    pass.name = full_pass_name;

                    MaterialPassMetadata pass_metadata{};
                    pass_metadata.data = pass_data;
                    material_metadatas.insert(full_pass_name, pass_metadata);

                    MaterialPassKey key = template_key;
                    key.material_pass_index = static_cast<uint32_t>(passes.size());

                    material_pass_keys.insert(full_pass_name, key);

                    passes.push_back(pass);
                }
            });
        });

        passes_by_pipeline.insert(pipeline.pipeline->name, passes);
    }

    void NovaRenderer::update_camera_matrix_buffer(const uint32_t frame_idx) {
        MTR_SCOPE("NovaRenderer", "update_camera_matrix_buffer");

        cameras.each_fwd([&](const Camera& cam) {
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
        });

        camera_data->upload_to_device(frame_idx);
    }

    rx::vector<rhi::RhiImage*> NovaRenderer::get_all_images(rx::memory::allocator& allocator) {
        rx::vector<rhi::RhiImage*> images{allocator};

        const auto& textures = device_resources->get_all_textures();
        images.reserve(textures.size());

        textures.each_fwd([&](const TextureResource& texture) { images.emplace_back(texture.image.get()); });

        return images;
    }

    void NovaRenderer::destroy_dynamic_resources() {
        MTR_SCOPE("destroy_dynamic_resources", "Self");
        if(loaded_renderpack) {
            loaded_renderpack->resources.render_targets.each_fwd([&](const renderpack::TextureCreateInfo& tex_data) {
                device_resources->destroy_render_target(tex_data.name);
            });

            logger->verbose("Deleted all dynamic textures from renderpack %s", loaded_renderpack->name);
        }
    }

    void NovaRenderer::destroy_renderpasses() {
        MTR_SCOPE("destroy_renderpasses", "Self");
        loaded_renderpack->graph_data.passes.each_fwd(
            [&](const renderpack::RenderPassCreateInfo& renderpass) { rendergraph->destroy_renderpass(renderpass.name); });
    }

    rhi::RhiSampler* NovaRenderer::get_point_sampler() const { return point_sampler.get(); }

    Pipeline* NovaRenderer::find_pipeline(const rx::string& pipeline_name) { return pipelines.find(pipeline_name); }

    RenderableId NovaRenderer::add_renderable_for_material(const FullMaterialPassName& material_name,
                                                           const StaticMeshRenderableCreateInfo& create_info) {
        MTR_SCOPE("add_renderable_for_material", "Self");
        const RenderableId id = next_renderable_id.load();
        next_renderable_id.fetch_add(1);

        const auto* pass_key = material_pass_keys.find(material_name);
        if(pass_key == nullptr) {
            logger->error("No material named %s for pass %s", material_name.material_name, material_name.pass_name);
            return std::numeric_limits<uint64_t>::max();
        }

        RenderableKey key;
        key.pipeline_name = pass_key->pipeline_name;
        key.material_pass_idx = pass_key->material_pass_index;

        // Figure out where to put the renderable
        auto* materials = passes_by_pipeline.find(pass_key->pipeline_name);

        if(!materials) {
            return std::numeric_limits<uint64_t>::max();
        }

        auto& material = (*materials)[pass_key->material_pass_index];

        StaticMeshRenderCommand command = make_render_command(create_info, id);

        if(const auto* mesh = meshes.find(create_info.mesh)) {
            if(create_info.is_static) {
                key.type = RenderableType::StaticMesh;
                bool need_to_add_batch = true;

                uint32_t batch_idx = 0;
                material.static_mesh_draws.each_fwd([&](MeshBatch<StaticMeshRenderCommand>& batch) {
                    if(batch.vertex_buffer == mesh->vertex_buffer.get()) {
                        key.batch_idx = batch_idx;
                        key.renderable_idx = static_cast<uint32_t>(batch.commands.size());

                        batch.commands.emplace_back(command);

                        need_to_add_batch = false;
                        return false;
                    }

                    batch_idx++;
                    return true;
                });

                if(need_to_add_batch) {
                    MeshBatch<StaticMeshRenderCommand> batch;
                    batch.num_vertex_attributes = mesh->num_vertex_attributes;
                    batch.num_indices = mesh->num_indices;
                    batch.vertex_buffer = mesh->vertex_buffer.get();
                    batch.index_buffer = mesh->index_buffer.get();
                    batch.commands.emplace_back(command);

                    key.batch_idx = static_cast<uint32_t>(material.static_mesh_draws.size());
                    key.renderable_idx = 0;

                    material.static_mesh_draws.emplace_back(batch);
                }
            }

        } else if(proc_meshes.find(create_info.mesh) != nullptr) {
            if(create_info.is_static) {
                key.type = RenderableType::ProceduralMesh;
                bool need_to_add_batch = false;

                uint32_t batch_idx = 0;
                material.static_procedural_mesh_draws.each_fwd([&](ProceduralMeshBatch<StaticMeshRenderCommand>& batch) {
                    if(batch.mesh.get_key() == create_info.mesh) {
                        key.batch_idx = batch_idx;
                        key.renderable_idx = static_cast<uint32_t>(batch.commands.size());

                        batch.commands.emplace_back(command);

                        need_to_add_batch = false;
                        return false;
                    }

                    batch_idx++;
                    return true;
                });

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

        renderable_keys.insert(id, key);

        return id;
    }

    void NovaRenderer::update_renderable(RenderableId renderable, const StaticMeshRenderableUpdateData& update_data) {
        MTR_SCOPE("NovaRenderer", "update_renderable");
        const auto* key = renderable_keys.find(renderable);
        if(key == nullptr) {
            logger->error("Could not update renderable %u", renderable);
            return;
        }

        auto* passes = passes_by_pipeline.find(key->pipeline_name);
        if(passes == nullptr) {
            logger->error("Could not find draws for pipeline %s, as requested by renderable %u", key->pipeline_name, renderable);
            return;
        }

        auto& material_pass = (*passes)[key->material_pass_idx];

        switch(key->type) {
            case RenderableType::StaticMesh: {
                auto& batch = material_pass.static_mesh_draws[key->batch_idx];
                auto& command = batch.commands[key->renderable_idx];

                command.is_visible = update_data.visible;

                // TODO: Make sure my matrix math is correct
                command.model_matrix = glm::translate(glm::mat4{1}, update_data.position);
                command.model_matrix = glm::scale(command.model_matrix, update_data.scale);
                command.model_matrix = glm::rotate(command.model_matrix, update_data.rotation.x, {1, 0, 0});
                command.model_matrix = glm::rotate(command.model_matrix, update_data.rotation.y, {0, 1, 0});
                command.model_matrix = glm::rotate(command.model_matrix, update_data.rotation.z, {0, 0, 1});

                break;
            }

            case RenderableType::ProceduralMesh: {
                auto& batch = material_pass.static_procedural_mesh_draws[key->batch_idx];
                auto& command = batch.commands[key->renderable_idx];

                command.is_visible = update_data.visible;

                // TODO: Make sure my matrix math is correct
                command.model_matrix = glm::translate(glm::mat4{1}, update_data.position);
                command.model_matrix = glm::scale(command.model_matrix, update_data.scale);
                command.model_matrix = glm::rotate(command.model_matrix, update_data.rotation.x, {1, 0, 0});
                command.model_matrix = glm::rotate(command.model_matrix, update_data.rotation.y, {0, 1, 0});
                command.model_matrix = glm::rotate(command.model_matrix, update_data.rotation.z, {0, 0, 1});

                break;
            }
        }
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

    void NovaRenderer::create_global_allocators() {

        // TODO: Make good
        renderpack_allocator = global_allocator;

        frame_allocators.reserve(settings->max_in_flight_frames);
        for(size_t i = 0; i < settings->max_in_flight_frames; i++) {
            rx_byte* ptr = global_allocator->allocate(PER_FRAME_MEMORY_SIZE.b_count());
            auto* mem = global_allocator->create<rx::memory::bump_point_allocator>(ptr, PER_FRAME_MEMORY_SIZE.b_count());
            frame_allocators.emplace_back(mem);
        }
    }

    void NovaRenderer::initialize_virtual_filesystem() {
        // The host application MUST register its data directory before initializing Nova

        const auto vfs = filesystem::VirtualFilesystem::get_instance();

        const auto renderpacks_directory = vfs->get_folder_accessor(RENDERPACK_DIRECTORY);

        vfs->add_resource_root(renderpacks_directory);
    }

    void NovaRenderer::create_global_sync_objects() {
        frame_fences = device->create_fences(settings->max_in_flight_frames, true, *global_allocator);
    }

    void NovaRenderer::create_global_samplers() {
        {
            // Default sampler create info will give us a delicious point sampler
            point_sampler = device->create_sampler({}, *global_allocator);
        }
    }

    void NovaRenderer::create_resource_storage() { device_resources = rx::make_ptr<DeviceResources>(*global_allocator, *this); }

    void NovaRenderer::create_builtin_render_targets() {
        const auto& swapchain_size = device->get_swapchain()->get_size();

        {
            const auto scene_output = device_resources->create_render_target(SCENE_OUTPUT_RT_NAME,
                                                                             swapchain_size.x,
                                                                             swapchain_size.y,
                                                                             rhi::PixelFormat::Rgba8,
                                                                             *global_allocator,
                                                                             true);

            if(!scene_output) {
                logger->error("Could not create scene output render target");

            } else {
                dynamic_texture_infos.insert(SCENE_OUTPUT_RT_NAME,
                                             {SCENE_OUTPUT_RT_NAME,
                                              renderpack::ImageUsage::RenderTarget,
                                              {rhi::PixelFormat::Rgba8, renderpack::TextureDimensionType::ScreenRelative, 1, 1}});
            }
        }

        {
            const auto ui_output = device_resources->create_render_target(UI_OUTPUT_RT_NAME,
                                                                          swapchain_size.x,
                                                                          swapchain_size.y,
                                                                          rhi::PixelFormat::Rgba8,
                                                                          *global_allocator,
                                                                          true);

            if(!ui_output) {
                logger->error("Could not create UI output render target");

            } else {
                dynamic_texture_infos.insert(UI_OUTPUT_RT_NAME,
                                             {UI_OUTPUT_RT_NAME,
                                              renderpack::ImageUsage::RenderTarget,
                                              {rhi::PixelFormat::Rgba8, renderpack::TextureDimensionType::ScreenRelative, 1, 1}});
            }
        }
    }

    void NovaRenderer::create_builtin_uniform_buffers() {
        auto* material_data_memory = global_allocator->allocate(MATERIAL_BUFFER_SIZE.b_count());
        material_buffer = rx::make_ptr<MaterialDataBuffer>(*global_allocator,
                                                           rx::memory::view{global_allocator,
                                                                            material_data_memory,
                                                                            MATERIAL_BUFFER_SIZE.b_count()});
        for(uint32_t i = 0; i < settings->max_in_flight_frames; i++) {
            const auto buffer_name = rx::string::format("%s_%d", MATERIAL_DATA_BUFFER_NAME, i);
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
        const static rx::array TRIANGLE_VERTICES{0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 2.0f};
        const static rx::array TRIANGLE_INDICES{0, 1, 2};

        const MeshData fullscreen_triangle_data{1,
                                                3,
                                                TRIANGLE_VERTICES.data(),
                                                TRIANGLE_VERTICES.size() * sizeof(float),
                                                TRIANGLE_INDICES.data(),
                                                TRIANGLE_INDICES.size() * sizeof(uint32_t)};

        fullscreen_triangle_id = create_mesh(fullscreen_triangle_data);
    }

    void NovaRenderer::create_renderpass_manager() { rendergraph = global_allocator->create<Rendergraph>(*global_allocator, *device); }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void NovaRenderer::create_builtin_renderpasses() {
        const auto& ui_output = *device_resources->get_render_target(UI_OUTPUT_RT_NAME);
        const auto& scene_output = *device_resources->get_render_target(SCENE_OUTPUT_RT_NAME);

        backbuffer_output_pipeline_create_info->viewport_size = device->get_swapchain()->get_size();
        auto backbuffer_pipeline = device->create_global_pipeline(*backbuffer_output_pipeline_create_info, *global_allocator);
        if(rendergraph->create_renderpass<BackbufferOutputRenderpass>(*device_resources,
                                                                      ui_output->image.get(),
                                                                      scene_output->image.get(),
                                                                      point_sampler.get(),
                                                                      rx::utility::move(backbuffer_pipeline),
                                                                      fullscreen_triangle_id,
                                                                      *device) == nullptr) {

            logger->error("Could not create the backbuffer output renderpass");
        }
    }

    void NovaRenderer::create_builtin_pipelines() {}

    void NovaRenderer::initialize_material_resource_binder() { material_resource_binder = device->get_material_resource_binder(); }
} // namespace nova::renderer
