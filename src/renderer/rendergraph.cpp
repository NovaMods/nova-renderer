#include "nova_renderer/rendergraph.hpp"

#include "nova_renderer/nova_renderer.hpp"
#include "nova_renderer/rhi/command_list.hpp"

#include "../loading/shaderpack/render_graph_builder.hpp"

namespace nova::renderer {
    using namespace shaderpack;

    Renderpass::Renderpass(rx::string name, const bool is_builtin) : name(std::move(name)), is_builtin(is_builtin) {}

    void Renderpass::render(rhi::CommandList& cmds, FrameContext& ctx) {
        // TODO: Figure if any of these barriers are implicit
        // TODO: Use shader reflection to figure our the stage that the pipelines in this renderpass need access to this resource instead of
        // using a robust default

        record_pre_renderpass_barriers(cmds, ctx);

        const auto framebuffer = get_framebuffer(ctx);

        cmds.begin_renderpass(renderpass, framebuffer);

        render_renderpass_contents(cmds, ctx);

        cmds.end_renderpass();

        record_post_renderpass_barriers(cmds, ctx);
    }

    void Renderpass::record_pre_renderpass_barriers(rhi::CommandList& cmds, FrameContext& ctx) const {
        if(read_texture_barriers.size() > 0) {
            // TODO: Use shader reflection to figure our the stage that the pipelines in this renderpass need access to this resource
            // instead of using a robust default
            cmds.resource_barriers(rhi::PipelineStage::ColorAttachmentOutput, rhi::PipelineStage::FragmentShader, read_texture_barriers);
        }

        if(write_texture_barriers.size() > 0) {
            // TODO: Use shader reflection to figure our the stage that the pipelines in this renderpass need access to this resource
            // instead of using a robust default
            cmds.resource_barriers(rhi::PipelineStage::ColorAttachmentOutput, rhi::PipelineStage::FragmentShader, write_texture_barriers);
        }

        if(writes_to_backbuffer) {
            rhi::ResourceBarrier backbuffer_barrier{};
            backbuffer_barrier.resource_to_barrier = ctx.swapchain_image;
            backbuffer_barrier.access_before_barrier = rhi::ResourceAccess::MemoryRead;
            backbuffer_barrier.access_after_barrier = rhi::ResourceAccess::ColorAttachmentWrite;
            backbuffer_barrier.old_state = rhi::ResourceState::PresentSource;
            backbuffer_barrier.new_state = rhi::ResourceState::RenderTarget;
            backbuffer_barrier.source_queue = rhi::QueueType::Graphics;
            backbuffer_barrier.destination_queue = rhi::QueueType::Graphics;
            backbuffer_barrier.image_memory_barrier.aspect = rhi::ImageAspect::Color;

            // TODO: Use shader reflection to figure our the stage that the pipelines in this renderpass need access to this resource
            // instead of using a robust default
            rx::vector<rhi::ResourceBarrier> barriers(&rx::memory::g_system_allocator, 1);
            barriers.push_back(backbuffer_barrier);
            cmds.resource_barriers(rhi::PipelineStage::TopOfPipe, rhi::PipelineStage::ColorAttachmentOutput, barriers);
        }
    }

    void Renderpass::render_renderpass_contents(rhi::CommandList& cmds, FrameContext& ctx) {
        auto& pipeline_storage = ctx.nova->get_pipeline_storage();

        // TODO: I _actually_ want to get all the draw commands from NovaRenderer, instead of storing them in this struct
        pipeline_names.each_fwd([&](const rx::string& pipeline_name) {
            const auto pipeline = pipeline_storage.get_pipeline(pipeline_name);
            if(pipeline) {
                pipeline->record(cmds, ctx);
            }
        });
    }

    void Renderpass::record_post_renderpass_barriers(rhi::CommandList& cmds, FrameContext& ctx) const {
        if(writes_to_backbuffer) {
            rhi::ResourceBarrier backbuffer_barrier{};
            backbuffer_barrier.resource_to_barrier = ctx.swapchain_image;
            backbuffer_barrier.access_before_barrier = rhi::ResourceAccess::ColorAttachmentWrite;
            backbuffer_barrier.access_after_barrier = rhi::ResourceAccess::MemoryRead;
            backbuffer_barrier.old_state = rhi::ResourceState::RenderTarget;
            backbuffer_barrier.new_state = rhi::ResourceState::PresentSource;
            backbuffer_barrier.source_queue = rhi::QueueType::Graphics;
            backbuffer_barrier.destination_queue = rhi::QueueType::Graphics;
            backbuffer_barrier.image_memory_barrier.aspect = rhi::ImageAspect::Color;

            rx::vector<rhi::ResourceBarrier> barriers(&rx::memory::g_system_allocator, 1);
            barriers.push_back(backbuffer_barrier);
            cmds.resource_barriers(rhi::PipelineStage::ColorAttachmentOutput, rhi::PipelineStage::BottomOfPipe, barriers);
        }
    }

    Rendergraph::Rendergraph(rx::memory::allocator* allocator, rhi::RenderDevice& device) : allocator(allocator), device(device) {}

    void Rendergraph::destroy_renderpass(const rx::string& name) {
        if(Renderpass** renderpass = renderpasses.find(name)) {
            device.destroy_framebuffer((*renderpass)->framebuffer, allocator);

            device.destroy_renderpass((*renderpass)->renderpass, allocator);

            renderpasses.erase(name);
            renderpass_metadatas.erase(name);

            is_dirty = true;
        }
    }

    rx::vector<rx::string> Rendergraph::calculate_renderpass_execution_order() {
        if(is_dirty) {
            // Oh look some bullshit I have to do because C++ doesn't have an API as cool as Java Streams
            const auto create_infos = [&]() {
                rx::vector<RenderPassCreateInfo> create_info_temp(allocator);
                create_info_temp.reserve(renderpass_metadatas.size());

                renderpass_metadatas.each_value([&](const RenderpassMetadata& metadata) { create_info_temp.emplace_back(metadata.data); });

                return create_info_temp;
            }();

            order_passes(create_infos)
                .map([&](const rx::vector<RenderPassCreateInfo>& order) {
                    cached_execution_order.clear();
                    cached_execution_order.reserve(order.size());
                    order.each_fwd([&](const RenderPassCreateInfo& create_info) { cached_execution_order.emplace_back(create_info.name); });

                    return true;
                })
                .on_error([](const auto& err) {
                    NOVA_LOG(ERROR) << "Could not determine renderpass execution order: " << err.to_string().data();
                });

            is_dirty = false;
        }

        return cached_execution_order;
    }

    Renderpass* Rendergraph::get_renderpass(const rx::string& name) const {
        if(Renderpass* const* renderpass = renderpasses.find(name)) {
            return *renderpass;
        }

        return nullptr;
    }

    rx::optional<RenderpassMetadata> Rendergraph::get_metadata_for_renderpass(const rx::string& name) const {
        if(const auto* metadata = renderpass_metadatas.find(name)) {
            return *metadata;
        }

        return rx::nullopt;
    }

    rhi::Framebuffer* Renderpass::get_framebuffer(const FrameContext& ctx) const {
        if(!writes_to_backbuffer) {
            return framebuffer;
        } else {
            return ctx.swapchain_framebuffer;
        }
    }

    void renderer::MaterialPass::record(rhi::CommandList& cmds, FrameContext& ctx) const {
        cmds.bind_descriptor_sets(descriptor_sets, pipeline_interface);

        static_mesh_draws.each_fwd(
            [&](const MeshBatch<StaticMeshRenderCommand>& batch) { record_rendering_static_mesh_batch(batch, cmds, ctx); });

        static_procedural_mesh_draws.each_fwd(
            [&](const ProceduralMeshBatch<StaticMeshRenderCommand>& batch) { record_rendering_static_mesh_batch(batch, cmds, ctx); });
    }

    void renderer::MaterialPass::record_rendering_static_mesh_batch(const MeshBatch<StaticMeshRenderCommand>& batch,
                                                                    rhi::CommandList& cmds,
                                                                    FrameContext& ctx) {
        const uint64_t start_index = ctx.cur_model_matrix_index;

        batch.commands.each_fwd([&](const StaticMeshRenderCommand& command) {
            if(command.is_visible) {
                auto model_matrix_buffer = ctx.nova->get_resource_manager().get_uniform_buffer(MODEL_MATRIX_BUFFER_NAME);
                ctx.nova->get_engine().write_data_to_buffer(&command.model_matrix,
                                                            sizeof(glm::mat4),
                                                            ctx.cur_model_matrix_index * sizeof(glm::mat4),
                                                            (*model_matrix_buffer)->buffer);
                ctx.cur_model_matrix_index++;
            }
        });

        if(start_index != ctx.cur_model_matrix_index) {
            // TODO: There's probably a better way to do this
            rx::vector<rhi::Buffer*> vertex_buffers(7);
            for(uint32_t i = 0; i < 7; i++) {
                vertex_buffers.push_back(batch.vertex_buffer);
            }
            cmds.bind_vertex_buffers(vertex_buffers);
            cmds.bind_index_buffer(batch.index_buffer);

            cmds.draw_indexed_mesh(static_cast<uint32_t>(batch.index_buffer->size.b_count() / sizeof(uint32_t)),
                                   static_cast<uint32_t>(ctx.cur_model_matrix_index - start_index));
        }
    }

    void renderer::MaterialPass::record_rendering_static_mesh_batch(const ProceduralMeshBatch<StaticMeshRenderCommand>& batch,
                                                                    rhi::CommandList& cmds,
                                                                    FrameContext& ctx) {
        const uint64_t start_index = ctx.cur_model_matrix_index;

        batch.commands.each_fwd([&](const StaticMeshRenderCommand& command) {
            if(command.is_visible) {
                auto model_matrix_buffer = ctx.nova->get_resource_manager().get_uniform_buffer(MODEL_MATRIX_BUFFER_NAME);
                ctx.nova->get_engine().write_data_to_buffer(&command.model_matrix,
                                                            sizeof(glm::mat4),
                                                            ctx.cur_model_matrix_index * sizeof(glm::mat4),
                                                            (*model_matrix_buffer)->buffer);
                ctx.cur_model_matrix_index++;
            }
        });

        if(start_index != ctx.cur_model_matrix_index) {
            const auto& [vertex_buffer, index_buffer] = batch.mesh->get_buffers_for_frame(ctx.frame_count % NUM_IN_FLIGHT_FRAMES);
            // TODO: There's probably a better way to do this
            rx::vector<rhi::Buffer*> vertex_buffers(7);
            for(uint32_t i = 0; i < 7; i++) {
                vertex_buffers.push_back(vertex_buffer);
            }
            cmds.bind_vertex_buffers(vertex_buffers);
            cmds.bind_index_buffer(index_buffer);
        }
    }

    void Pipeline::record(rhi::CommandList& cmds, FrameContext& ctx) const {
        cmds.bind_pipeline(pipeline);

        const auto& passes = ctx.nova->get_material_passes_for_pipeline(pipeline);

        passes.each_fwd([&](const renderer::MaterialPass& pass) { pass.record(cmds, ctx); });
    }
} // namespace nova::renderer
