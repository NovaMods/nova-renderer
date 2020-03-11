#include "nova_renderer/nova_renderer.hpp"

#include <array>
#include <future>

#pragma warning(push, 0)
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <minitrace.h>
#include <spirv_glsl.hpp>
#pragma warning(pop)

#include <glslang/MachineIndependent/Initialize.h>
#include <rx/core/array.h>
#include <rx/core/global.h>
#include <rx/core/log.h>
#include <rx/core/memory/bump_point_allocator.h>

#include "nova_renderer/constants.hpp"
#include "nova_renderer/loading/renderpack_loading.hpp"
#include "nova_renderer/memory/block_allocation_strategy.hpp"
#include "nova_renderer/memory/bump_point_allocation_strategy.hpp"
#include "nova_renderer/procedural_mesh.hpp"
#include "nova_renderer/rendergraph.hpp"
#include "nova_renderer/renderpack_data_conversions.hpp"
#include "nova_renderer/rhi/command_list.hpp"
#include "nova_renderer/rhi/swapchain.hpp"
#include "nova_renderer/ui_renderer.hpp"
#include "nova_renderer/util/platform.hpp"

#include "debugging/renderdoc.hpp"
#include "loading/renderpack/render_graph_builder.hpp"
#include "render_objects/uniform_structs.hpp"
#include "renderer/builtin/backbuffer_output_pass.hpp"
#include "rhi/vulkan/vulkan_render_device.hpp"
using namespace nova::mem;
using namespace operators;

rx::global_group g_nova_globals{"Nova"};

RX_LOG("nova", logger);

// TODO: Use this somehow
const Bytes GLOBAL_MEMORY_POOL_SIZE = 1_gb;

rx::global<nova::renderer::LogHandles> logging_event_handles{"system", "log_handles", &rx::memory::g_system_allocator};

void init_rex() {
    static bool initialized = false;

    if(!initialized) {
        rx::globals::link();

        rx::global_group* system_group{rx::globals::find("system")};

        // Explicitly initialize globals that need to be initialized in a specific
        // order for things to work.
        system_group->find("allocator")->init();
        system_group->find("logger")->init();

        auto* log_handles_global = system_group->find("log_handles");
        log_handles_global->init();

        auto* log_handles = log_handles_global->cast<nova::renderer::LogHandles>();

        rx::globals::find("loggers")->each([&](rx::global_node* logger_node) {
            log_handles->push_back(logger_node->cast<rx::log>()->on_write([](const rx::log::level level, const rx::string& message) {
                switch(level) {
                    case rx::log::level::k_error:
                        printf("[error  ]: %s\n", message.data());
                        break;
                    case rx::log::level::k_info:
                        printf("[info   ]: %s\n", message.data());
                        break;
                    case rx::log::level::k_verbose:
                        printf("[verbose]: %s\n", message.data());
                        break;
                    case rx::log::level::k_warning:
                        printf("[warning]: %s\n", message.data());
                        break;
                }
            }));
        });

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
        system_group->find("allocator")->fini();

        deinitialized = true;
    }
}

namespace nova::renderer {
    struct RX_HINT_EMPTY_BASES BackbufferOutputPipelineCreateInfo : PipelineStateCreateInfo {
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
            logger(rx::log::level::k_error, "Could not compile builtin backbuffer output vertex shader");
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
        const auto& pixel_spirv = renderpack::compile_shader(pixel_source, rhi::ShaderStage::Fragment, rhi::ShaderLanguage::Hlsl);
        if(pixel_spirv.is_empty()) {
            logger(rx::log::level::k_error, "Could not compile builtin backbuffer output pixel shader");
        }
        pixel_shader = {"/nova/shaders/backbuffer_output.pixel.hlsl", pixel_spirv};

        vertex_fields.emplace_back("position", rhi::VertexFieldFormat::Float2);

        // TODO: Figure out how to make the input textures into input attachments
        color_attachments.emplace_back(BACKBUFFER_NAME, rhi::PixelFormat::Rgba8, false);
    }

    rx::global<BackbufferOutputPipelineCreateInfo> backbuffer_output_pipeline_create_info{"Nova", "BackbufferOutputPipelineCreateInfo"};

    bool FullMaterialPassName::operator==(const FullMaterialPassName& other) const {
        return material_name == other.material_name && pass_name == other.pass_name;
    }

    rx_size FullMaterialPassName::hash() const {
        const rx_size material_name_hash = rx::hash<rx::string>{}(material_name);
        const rx_size pass_name_hash = rx::hash<rx::string>{}(pass_name);

        return material_name_hash ^ pass_name_hash;
    }

    NovaRenderer::NovaRenderer(const NovaSettings& settings) : render_settings(settings) {
        create_global_allocators();

        initialize_virtual_filesystem();

        mtr_init("trace.json");

        MTR_META_PROCESS_NAME("NovaRenderer");
        MTR_META_THREAD_NAME("Main");

        MTR_SCOPE("Init", "nova_renderer::nova_renderer");

        window = std::make_unique<NovaWindow>(settings);

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

                    rg_log(rx::log::level::k_info, "Loaded RenderDoc successfully");

                    return 0;
                })
                .on_error([](const ntl::NovaError& error) { rg_log(rx::log::level::k_error, "%s", error.to_string()); });
        }

        {
            MTR_SCOPE("Init", "InitVulkanRenderDevice");
            device = std::make_unique<rhi::VulkanRenderDevice>(render_settings, *window, global_allocator);
        }

        swapchain = device->get_swapchain();

        create_global_gpu_pools();

        create_global_sync_objects();

        create_global_samplers();

        create_resource_storage();

        create_builtin_render_targets();

        create_builtin_uniform_buffers();

        create_builtin_meshes();

        create_renderpass_manager();

        initialize_descriptor_pool();

        create_builtin_renderpasses();
    }

    NovaRenderer::~NovaRenderer() { mtr_shutdown(); }

    NovaSettingsAccessManager& NovaRenderer::get_settings() { return render_settings; }

    rx::memory::allocator* NovaRenderer::get_global_allocator() const { return global_allocator; }

    void NovaRenderer::execute_frame() {
        MTR_SCOPE("RenderLoop", "execute_frame");
        frame_count++;

        rx::memory::bump_point_allocator* frame_allocator = frame_allocators[frame_count % NUM_IN_FLIGHT_FRAMES];
        frame_allocator->reset();

        cur_frame_idx = device->get_swapchain()->acquire_next_swapchain_image(frame_allocator);

        rx::vector<rhi::RhiFence*> cur_frame_fences{global_allocator};
        cur_frame_fences.push_back(frame_fences[cur_frame_idx]);

        device->wait_for_fences(cur_frame_fences);
        device->reset_fences(cur_frame_fences);

        rhi::CommandList* cmds = device->create_command_list(0,
                                                             rhi::QueueType::Graphics,
                                                             rhi::CommandList::Level::Primary,
                                                             frame_allocator);
        cmds->set_debug_name("RendergraphCommands");

        FrameContext ctx = {};
        ctx.frame_count = frame_count;
        ctx.nova = this;
        ctx.allocator = frame_allocator;
        ctx.swapchain_framebuffer = swapchain->get_framebuffer(cur_frame_idx);
        ctx.swapchain_image = swapchain->get_image(cur_frame_idx);

        const auto& renderpass_order = rendergraph->calculate_renderpass_execution_order();

        renderpass_order.each_fwd([&](const rx::string& renderpass_name) {
            auto* renderpass = rendergraph->get_renderpass(renderpass_name);
            renderpass->execute(*cmds, ctx);
        });

        device->submit_command_list(cmds, rhi::QueueType::Graphics, frame_fences[cur_frame_idx]);

        // Wait for the GPU to finish before presenting. This destroys pipelining and throughput, however at this time I'm not sure how
        // best to say "when GPU finishes this task, CPU should do something"
        device->wait_for_fences(cur_frame_fences);

        device->get_swapchain()->present(cur_frame_idx);

        mtr_flush();
    }

    void NovaRenderer::set_num_meshes(const uint32_t /* num_meshes */) { /* TODO? */
    }

    MeshId NovaRenderer::create_mesh(const MeshData& mesh_data) {
        if(mesh_data.num_vertex_attributes == 0) {
            logger(rx::log::level::k_error, "Can not add a mesh with zero vertex attributes");
        }

        if(mesh_data.num_indices == 0) {
            logger(rx::log::level::k_error, "Can not add a mesh with zero indices");
        }

        rhi::RhiBufferCreateInfo vertex_buffer_create_info;
        vertex_buffer_create_info.buffer_usage = rhi::BufferUsage::VertexBuffer;
        vertex_buffer_create_info.size = mesh_data.vertex_data_size;

        rhi::RhiBuffer* vertex_buffer = device->create_buffer(vertex_buffer_create_info, *mesh_memory, global_allocator);

        // TODO: Try to get staging buffers from a pool

        {
            rhi::RhiBufferCreateInfo staging_vertex_buffer_create_info = vertex_buffer_create_info;
            staging_vertex_buffer_create_info.buffer_usage = rhi::BufferUsage::StagingBuffer;

            rhi::RhiBuffer* staging_vertex_buffer = device->create_buffer(staging_vertex_buffer_create_info,
                                                                          *staging_buffer_memory,
                                                                          global_allocator);
            device->write_data_to_buffer(mesh_data.vertex_data_ptr, vertex_buffer_create_info.size, 0, staging_vertex_buffer);

            rhi::CommandList* vertex_upload_cmds = device->create_command_list(0,
                                                                               rhi::QueueType::Transfer,
                                                                               rhi::CommandList::Level::Primary,
                                                                               global_allocator);
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

            rx::vector<rhi::RhiResourceBarrier> barriers{global_allocator};
            barriers.push_back(vertex_barrier);
            vertex_upload_cmds->resource_barriers(rhi::PipelineStage::Transfer, rhi::PipelineStage::VertexInput, barriers);

            device->submit_command_list(vertex_upload_cmds, rhi::QueueType::Transfer);

            // TODO: Barrier on the mesh's first usage
        }

        rhi::RhiBufferCreateInfo index_buffer_create_info;
        index_buffer_create_info.buffer_usage = rhi::BufferUsage::IndexBuffer;
        index_buffer_create_info.size = mesh_data.index_data_size;

        rhi::RhiBuffer* index_buffer = device->create_buffer(index_buffer_create_info, *mesh_memory, global_allocator);

        {
            rhi::RhiBufferCreateInfo staging_index_buffer_create_info = index_buffer_create_info;
            staging_index_buffer_create_info.buffer_usage = rhi::BufferUsage::StagingBuffer;
            rhi::RhiBuffer* staging_index_buffer = device->create_buffer(staging_index_buffer_create_info,
                                                                         *staging_buffer_memory,
                                                                         global_allocator);
            device->write_data_to_buffer(mesh_data.index_data_ptr, index_buffer_create_info.size, 0, staging_index_buffer);

            rhi::CommandList* indices_upload_cmds = device->create_command_list(0,
                                                                                rhi::QueueType::Transfer,
                                                                                rhi::CommandList::Level::Primary,
                                                                                global_allocator);
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

            rx::vector<rhi::RhiResourceBarrier> barriers{global_allocator};
            barriers.push_back(index_barrier);
            indices_upload_cmds->resource_barriers(rhi::PipelineStage::Transfer, rhi::PipelineStage::VertexInput, barriers);

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
        meshes.insert(new_mesh_id, mesh);

        return new_mesh_id;
    }

    ProceduralMeshAccessor NovaRenderer::create_procedural_mesh(const uint64_t vertex_size, const uint64_t index_size) {
        const MeshId our_id = next_mesh_id;
        next_mesh_id++;

        proc_meshes.insert(our_id, ProceduralMesh{vertex_size, index_size, device.get()});

        return ProceduralMeshAccessor{&proc_meshes, our_id};
    }

    void NovaRenderer::load_renderpack(const rx::string& renderpack_name) {
        MTR_SCOPE("RenderpackLoading", "load_renderpack");
        glslang::InitializeProcess();

        const renderpack::RenderpackData data = renderpack::load_renderpack_data(renderpack_name);

        if(renderpacks_loaded) {
            destroy_dynamic_resources();

            destroy_renderpasses();
            rg_log(rx::log::level::k_verbose, "Resources from old renderpack destroyed");
        }

        create_dynamic_textures(data.resources.render_targets);
        rg_log(rx::log::level::k_verbose, "Dynamic textures created");

        create_render_passes(data.graph_data.passes, data.pipelines);

        rg_log(rx::log::level::k_verbose, "Created render passes");

        create_pipelines_and_materials(data.pipelines, data.materials);

        rg_log(rx::log::level::k_verbose, "Created pipelines and materials");

        renderpacks_loaded = true;

        rg_log(rx::log::level::k_verbose, "Renderpack %s loaded successfully", renderpack_name);
    }

    const rx::vector<MaterialPass>& NovaRenderer::get_material_passes_for_pipeline(rhi::RhiPipeline* const pipeline) {
        return *passes_by_pipeline.find(pipeline);
    }

    rx::optional<RenderpassMetadata> NovaRenderer::get_renderpass_metadata(const rx::string& renderpass_name) const {
        return rendergraph->get_metadata_for_renderpass(renderpass_name);
    }

    void NovaRenderer::create_dynamic_textures(const rx::vector<renderpack::TextureCreateInfo>& texture_create_infos) {
        texture_create_infos.each_fwd([&](const renderpack::TextureCreateInfo& create_info) {
            const auto size = create_info.format.get_size_in_pixels(device->get_swapchain()->get_size());

            const auto render_target = device_resources->create_render_target(create_info.name,
                                                                              size.x,
                                                                              size.y,
                                                                              create_info.format.pixel_format,
                                                                              renderpack_allocator);

            dynamic_texture_infos.insert(create_info.name, create_info);
        });
    }

    void NovaRenderer::create_render_passes(const rx::vector<renderpack::RenderPassCreateInfo>& pass_create_infos,
                                            const rx::vector<renderpack::PipelineData>& pipelines) const {

        device->set_num_renderpasses(static_cast<uint32_t>(pass_create_infos.size()));

        pass_create_infos.each_fwd([&](const renderpack::RenderPassCreateInfo& create_info) {
            auto* renderpass = global_allocator->create<Renderpass>(create_info.name);
            if(rendergraph->add_renderpass(renderpass, create_info, *device_resources) != nullptr) {
                pipelines.each_fwd([&](const renderpack::PipelineData& pipeline) {
                    if(pipeline.pass == create_info.name) {
                        renderpass->pipeline_names.emplace_back(pipeline.name);
                    }
                });
            } else {
                rg_log(rx::log::level::k_error, "Could not create renderpass %s", create_info.name);
            }
        });
    }

    void NovaRenderer::create_pipelines_and_materials(const rx::vector<renderpack::PipelineData>& pipeline_create_infos,
                                                      const rx::vector<renderpack::MaterialData>& materials) {
        pipeline_create_infos.each_fwd([&](const renderpack::PipelineData& pipeline_create_info) {
            const auto pipeline_state_create_info = renderpack::to_pipeline_state_create_info(pipeline_create_info, *rendergraph);
            if(!pipeline_state_create_info) {
                logger(rx ::log::level::k_error, "Could not create pipeline %s", pipeline_create_info.name);
            }

            if(pipeline_storage->create_pipeline(*pipeline_state_create_info)) {
                auto pipeline = pipeline_storage->get_pipeline(pipeline_state_create_info->name);
                create_materials_for_pipeline(*pipeline, materials, pipeline_create_info.name);
            }
        });
    }

    void NovaRenderer::create_materials_for_pipeline(const Pipeline& pipeline,
                                                     const rx::vector<renderpack::MaterialData>& materials,
                                                     const rx::string& pipeline_name) {

        // Determine the pipeline layout so the material can create descriptors for the pipeline

        MaterialPassKey template_key = {};
        template_key.pipeline_name = pipeline_name;

        // Large overestimate, but that's fine
        rx::vector<MaterialPass> passes;
        passes.reserve(materials.size());

        materials.each_fwd([&](const renderpack::MaterialData& material_data) {
            material_data.passes.each_fwd([&](const renderpack::MaterialPass& pass_data) {
                if(pass_data.pipeline == pipeline_name) {
                    MaterialPass pass = {};
                    pass.pipeline_interface = pipeline.pipeline_interface;

                    if(!pipeline.pipeline_interface->bindings.is_empty()) {
                        pass.descriptor_sets = device->create_descriptor_sets(pipeline.pipeline_interface,
                                                                              global_descriptor_pool,
                                                                              renderpack_allocator);
                    }

                    bind_data_to_material_descriptor_sets(pass, pass_data.bindings, pipeline.pipeline_interface->bindings);

                    const FullMaterialPassName full_pass_name{pass_data.material_name, pass_data.name};

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

        passes_by_pipeline.insert(pipeline.pipeline, passes);
    }

    void NovaRenderer::bind_data_to_material_descriptor_sets(
        const MaterialPass& material,
        const rx::map<rx::string, rx::string>& bindings,
        const rx::map<rx::string, rhi::RhiResourceBindingDescription>& descriptor_descriptions) {

        rx::vector<rhi::RhiDescriptorSetWrite> writes;
        writes.reserve(bindings.size());

        bindings.each_pair([&](const rx::string& descriptor_name, const rx::string& resource_name) {
            const rhi::RhiResourceBindingDescription& binding_desc = *descriptor_descriptions.find(descriptor_name);
            rhi::RhiDescriptorSet* descriptor_set = material.descriptor_sets[binding_desc.set];

            rhi::RhiDescriptorSetWrite write = {};
            write.set = descriptor_set;
            write.binding = binding_desc.binding;
            write.resources.emplace_back();
            rhi::RhiDescriptorResourceInfo& resource_info = write.resources[0];

            if(const auto dyn_tex = device_resources->get_render_target(resource_name); dyn_tex) {
                rhi::RhiImage* image = (*dyn_tex)->image;

                resource_info.image_info.image = image;
                resource_info.image_info.format = dynamic_texture_infos.find(resource_name)->format;

                write.type = rhi::DescriptorType::Texture;

                writes.push_back(write);

            } else if(const auto buffer_access = device_resources->get_uniform_buffer(resource_name); buffer_access) {
                rhi::RhiBuffer* buffer = (*buffer_access)->buffer;

                resource_info.buffer_info.buffer = buffer;
                write.type = rhi::DescriptorType::UniformBuffer;

                writes.push_back(write);

            } else if(resource_name == POINT_SAMPLER_NAME) {
                resource_info.sampler_info.sampler = point_sampler;
                write.type = rhi::DescriptorType::Sampler;

                writes.push_back(write);

            } else {
                rg_log(rx::log::level::k_error, "Resource %s is not known to Nova", resource_name);
            }
        });

        device->update_descriptor_sets(writes);
    }

    void NovaRenderer::destroy_dynamic_resources() {
        if(loaded_renderpack) {
            loaded_renderpack->resources.render_targets.each_fwd([&](const renderpack::TextureCreateInfo& tex_data) {
                device_resources->destroy_render_target(tex_data.name, renderpack_allocator);
            });

            rg_log(rx::log::level::k_verbose, "Deleted all dynamic textures from renderpack %s", loaded_renderpack->name);
        }
    }

    void NovaRenderer::destroy_renderpasses() {
        loaded_renderpack->graph_data.passes.each_fwd(
            [&](const renderpack::RenderPassCreateInfo& renderpass) { rendergraph->destroy_renderpass(renderpass.name); });
    }

    rhi::RhiSampler* NovaRenderer::get_point_sampler() const { return point_sampler; }

    RenderableId NovaRenderer::add_renderable_for_material(const FullMaterialPassName& material_name,
                                                           const StaticMeshRenderableData& renderable) {
        const RenderableId id = next_renderable_id.load();
        next_renderable_id.fetch_add(1);

        const auto* pass_key = material_pass_keys.find(material_name);
        if(pass_key == nullptr) {
            rg_log(rx::log::level::k_error, "No material named %s for pass %s", material_name.material_name, material_name.pass_name);
            return std::numeric_limits<uint64_t>::max();
        }

        MaterialPass material = {};

        StaticMeshRenderCommand command = make_render_command(renderable, id);

        if(const auto* mesh = meshes.find(renderable.mesh)) {
            if(renderable.is_static) {
                bool need_to_add_batch = true;

                material.static_mesh_draws.each_fwd([&](MeshBatch<StaticMeshRenderCommand>& batch) {
                    if(batch.vertex_buffer == mesh->vertex_buffer) {
                        batch.commands.emplace_back(command);

                        need_to_add_batch = false;
                        return false;
                    }
                    return true;
                });

                if(need_to_add_batch) {
                    MeshBatch<StaticMeshRenderCommand> batch;
                    batch.num_vertex_attributes = mesh->num_vertex_attributes;
                    batch.num_indices = mesh->num_indices;
                    batch.vertex_buffer = mesh->vertex_buffer;
                    batch.index_buffer = mesh->index_buffer;
                    batch.commands.emplace_back(command);

                    material.static_mesh_draws.emplace_back(batch);
                }
            }

        } else if(const auto* proc_mesh = proc_meshes.find(renderable.mesh)) {
            // TODO: Look at this when you're less frazzled
            if(renderable.is_static) {
                bool need_to_add_batch = false;

                material.static_procedural_mesh_draws.each_fwd([&](ProceduralMeshBatch<StaticMeshRenderCommand>& batch) {
                    if(batch.mesh.get_key() == renderable.mesh) {
                        batch.commands.emplace_back(command);

                        need_to_add_batch = false;
                        return false;
                    }

                    return true;
                });

                if(need_to_add_batch) {
                    ProceduralMeshBatch<StaticMeshRenderCommand> batch(&proc_meshes, renderable.mesh);
                    batch.commands.emplace_back(command);

                    material.static_procedural_mesh_draws.emplace_back(batch);
                }
            }
        } else {
            logger(rx::log::level::k_error, "Could not find a mesh with ID %u", renderable.mesh);
        }

        // Figure out where to put the renderable
        const auto pipeline = pipeline_storage->get_pipeline(pass_key->pipeline_name);
        if(pipeline) {
            auto* passes = passes_by_pipeline.find(pipeline->pipeline);
            passes->emplace_back(material);

        } else {
            logger(rx::log::level::k_error, "Could not get place the new renderable in the appropriate draw command list");
        }

        return id;
    }

    rhi::RenderDevice& NovaRenderer::get_engine() const { return *device; }

    NovaWindow& NovaRenderer::get_window() const { return *window; }

    DeviceResources& NovaRenderer::get_resource_manager() const { return *device_resources; }

    PipelineStorage& NovaRenderer::get_pipeline_storage() const { return *pipeline_storage; }

    void NovaRenderer::create_global_allocators() {
        global_allocator = &rx::memory::g_system_allocator;

        // TODO: Make good
        renderpack_allocator = global_allocator;

        frame_allocators.reserve(NUM_IN_FLIGHT_FRAMES);
        for(size_t i = 0; i < NUM_IN_FLIGHT_FRAMES; i++) {
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

    void NovaRenderer::create_global_gpu_pools() {
        const uint64_t mesh_memory_size = 512000000;
        ntl::Result<rhi::RhiDeviceMemory*> memory_result = device->allocate_device_memory(mesh_memory_size,
                                                                                          rhi::MemoryUsage::DeviceOnly,
                                                                                          rhi::ObjectType::Buffer,
                                                                                          global_allocator);
        const ntl::Result<DeviceMemoryResource*> mesh_memory_result = memory_result.map([&](rhi::RhiDeviceMemory* memory) {
            auto* allocator = global_allocator->create<BlockAllocationStrategy>(global_allocator, Bytes(mesh_memory_size), 64_b);
            return global_allocator->create<DeviceMemoryResource>(memory, allocator);
        });

        if(mesh_memory_result) {
            mesh_memory = global_allocator->create<DeviceMemoryResource>(*mesh_memory_result.value);

        } else {
            logger(rx::log::level::k_error, "Could not create mesh memory pool: %s", mesh_memory_result.error.to_string());
        }

        // Assume 65k things, plus we need space for the builtin ubos
        const uint64_t ubo_memory_size = sizeof(PerFrameUniforms) + sizeof(glm::mat4) * 0xFFFF;
        const ntl::Result<DeviceMemoryResource*>
            ubo_memory_result = device
                                    ->allocate_device_memory(ubo_memory_size,
                                                             rhi::MemoryUsage::DeviceOnly,
                                                             rhi::ObjectType::Buffer,
                                                             global_allocator)
                                    .map([=](rhi::RhiDeviceMemory* memory) {
                                        auto* allocator = global_allocator->create<BumpPointAllocationStrategy>(Bytes(ubo_memory_size),
                                                                                                                Bytes(sizeof(glm::mat4)));
                                        return global_allocator->create<DeviceMemoryResource>(memory, allocator);
                                    });

        if(ubo_memory_result) {
            ubo_memory = global_allocator->create<DeviceMemoryResource>(*ubo_memory_result.value);

        } else {
            logger(rx::log::level::k_error, "Could not create mesh memory pool: %s", ubo_memory_result.error.to_string());
        }

        // Staging buffers will be pooled, so we don't need a _ton_ of memory for them
        const Bytes staging_memory_size = 256_kb;
        const ntl::Result<DeviceMemoryResource*>
            staging_memory_result = device
                                        ->allocate_device_memory(staging_memory_size.b_count(),
                                                                 rhi::MemoryUsage::StagingBuffer,
                                                                 rhi::ObjectType::Buffer,
                                                                 global_allocator)
                                        .map([=](rhi::RhiDeviceMemory* memory) {
                                            auto* allocator = global_allocator->create<BumpPointAllocationStrategy>(staging_memory_size,
                                                                                                                    64_b);
                                            return global_allocator->create<DeviceMemoryResource>(memory, allocator);
                                        });

        if(staging_memory_result) {
            staging_buffer_memory = global_allocator->create<DeviceMemoryResource>(*staging_memory_result.value);

        } else {
            logger(rx::log::level::k_error, "Could not create staging buffer memory pool: %s", staging_memory_result.error.to_string());
        }
    }

    void NovaRenderer::create_global_sync_objects() {
        const rx::vector<rhi::RhiFence*>& fences = device->create_fences(NUM_IN_FLIGHT_FRAMES, true, global_allocator);
        for(uint32_t i = 0; i < NUM_IN_FLIGHT_FRAMES; i++) {
            frame_fences[i] = fences[i];
        }
    }

    void NovaRenderer::create_global_samplers() {
        {
            // Default sampler create info will give us a delicious point sampler
            point_sampler = device->create_sampler({}, global_allocator);
        }
    }

    void NovaRenderer::create_resource_storage() {
        device_resources = global_allocator->create<DeviceResources>(*this);

        pipeline_storage = global_allocator->create<PipelineStorage>(*this, global_allocator);
    }

    void NovaRenderer::create_builtin_render_targets() {
        const auto& swapchain_size = device->get_swapchain()->get_size();

        {
            const auto scene_output = device_resources->create_render_target(SCENE_OUTPUT_RT_NAME,
                                                                             swapchain_size.x,
                                                                             swapchain_size.y,
                                                                             rhi::PixelFormat::Rgba8,
                                                                             global_allocator,
                                                                             true);

            if(!scene_output) {
                logger(rx::log::level::k_error, "Could not create scene output render target");

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
                                                                          global_allocator,
                                                                          true);

            if(!ui_output) {
                logger(rx::log::level::k_error, "Could not create UI output render target");

            } else {
                dynamic_texture_infos.insert(UI_OUTPUT_RT_NAME,
                                             {UI_OUTPUT_RT_NAME,
                                              renderpack::ImageUsage::RenderTarget,
                                              {rhi::PixelFormat::Rgba8, renderpack::TextureDimensionType::ScreenRelative, 1, 1}});
            }
        }
    }

    void NovaRenderer::create_builtin_uniform_buffers() {
        if(device_resources->create_uniform_buffer(PER_FRAME_DATA_NAME, sizeof(PerFrameUniforms))) {
            builtin_buffer_names.emplace_back(PER_FRAME_DATA_NAME);

        } else {
            logger(rx::log::level::k_error, "Could not create builtin buffer ", PER_FRAME_DATA_NAME);
        }

        if(device_resources->create_uniform_buffer(MODEL_MATRIX_BUFFER_NAME, sizeof(glm::mat4) * 0xFFFF)) {
            builtin_buffer_names.emplace_back(MODEL_MATRIX_BUFFER_NAME);

        } else {
            logger(rx::log::level::k_error, "Could not create builtin buffer %s", MODEL_MATRIX_BUFFER_NAME);
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

    void NovaRenderer::create_renderpass_manager() { rendergraph = global_allocator->create<Rendergraph>(global_allocator, *device); }

    void NovaRenderer::create_builtin_renderpasses() {
        const auto& ui_output = *device_resources->get_render_target(UI_OUTPUT_RT_NAME);
        const auto& scene_output = *device_resources->get_render_target(SCENE_OUTPUT_RT_NAME);

        if(rendergraph->create_renderpass<BackbufferOutputRenderpass>(*device_resources, ui_output->image, scene_output->image) ==
           nullptr) {
            logger(rx::log::level::k_error, "Could not create the backbuffer output renderpass");
        }

        backbuffer_output_pipeline_create_info->viewport_size = device->get_swapchain()->get_size();
        if(!pipeline_storage->create_pipeline(*backbuffer_output_pipeline_create_info)) {
            logger(rx::log::level::k_error, "Could not create builtin pipeline %s", backbuffer_output_pipeline_create_info->name);

        } else {
            const auto pipeline = pipeline_storage->get_pipeline(backbuffer_output_pipeline_create_info->name);

            const renderpack::MaterialData material{BACKBUFFER_OUTPUT_MATERIAL_NAME,
                                                    rx::array{
                                                        renderpack::MaterialPass{"main",
                                                                                 BACKBUFFER_OUTPUT_MATERIAL_NAME,
                                                                                 BACKBUFFER_OUTPUT_PIPELINE_NAME,
                                                                                 rx::array{rx::pair{"ui_output", UI_OUTPUT_RT_NAME},
                                                                                           rx::pair{"scene_output", SCENE_OUTPUT_RT_NAME},
                                                                                           rx::pair{"tex_sampler", POINT_SAMPLER_NAME}},
                                                                                 {}}},
                                                    "block"};

            const rx::vector<renderpack::MaterialData> materials = rx::array{material};
            create_materials_for_pipeline(*pipeline, materials, backbuffer_output_pipeline_create_info->name);

            const static FullMaterialPassName BACKBUFFER_OUTPUT_MATERIAL{BACKBUFFER_OUTPUT_MATERIAL_NAME, "main"};
            const static StaticMeshRenderableData FULLSCREEN_TRIANGLE_RENDERABLE{{fullscreen_triangle_id}};

            backbuffer_output_renderable = add_renderable_for_material(BACKBUFFER_OUTPUT_MATERIAL, FULLSCREEN_TRIANGLE_RENDERABLE);
        }
    }

    void NovaRenderer::initialize_descriptor_pool() {
        global_descriptor_pool = device->create_descriptor_pool(rx::array{rx::pair{rhi::DescriptorType::UniformBuffer, 4096},
                                                                          rx::pair{rhi::DescriptorType::CombinedImageSampler, 4096},
                                                                          rx::pair{rhi::DescriptorType::Sampler, 5}},
                                                                global_allocator);
    }

    void NovaRenderer::create_builtin_pipelines() {}
} // namespace nova::renderer
