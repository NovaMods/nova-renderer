#include "nova_renderer/rendergraph.hpp"

#include <utility>

#include <minitrace.h>

#include "nova_renderer/nova_renderer.hpp"
#include "nova_renderer/rhi/command_list.hpp"

#include "../loading/renderpack/render_graph_builder.hpp"
#include "pipeline_reflection.hpp"

namespace nova::renderer {
    using namespace renderpack;

    RX_LOG("Rendergraph", logger);

    Renderpass::Renderpass(rx::string name, const bool is_builtin) : name(std::move(name)), is_builtin(is_builtin) {}

    void Renderpass::execute(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) {
        const auto& profiling_event_name = rx::string::format("Execute %s", name);
        MTR_SCOPE("Renderpass", profiling_event_name.data());
        // TODO: Figure if any of these barriers are implicit
        // TODO: Use shader reflection to figure our the stage that the pipelines in this renderpass need access to this resource instead of
        // using a robust default

        record_pre_renderpass_barriers(cmds, ctx);

        setup_renderpass(cmds, ctx);

        const auto framebuffer = get_framebuffer(ctx);

        cmds.begin_renderpass(renderpass, framebuffer);

        record_renderpass_contents(cmds, ctx);

        cmds.end_renderpass();

        record_post_renderpass_barriers(cmds, ctx);
    }

    void Renderpass::record_pre_renderpass_barriers(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) const {
        MTR_SCOPE("Renderpass", "record_pre_renderpass_barriers");
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
            rhi::RhiResourceBarrier backbuffer_barrier{};
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
            rx::vector<rhi::RhiResourceBarrier> barriers{&rx::memory::g_system_allocator};
            barriers.push_back(backbuffer_barrier);
            cmds.resource_barriers(rhi::PipelineStage::TopOfPipe, rhi::PipelineStage::ColorAttachmentOutput, barriers);
        }
    }

    void Renderpass::record_renderpass_contents(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) {
        MTR_SCOPE("Renderpass", "record_renderpass_contents");

        pipeline_names.each_fwd([&](const rx::string& pipeline_name) {
            const auto* pipeline = ctx.nova->find_pipeline(pipeline_name);
            if(pipeline) {
                pipeline->record(cmds, ctx);
            }
        });
    }

    void Renderpass::record_post_renderpass_barriers(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) const {
        MTR_SCOPE("Renderpass", "record_post_renderpass_barriers");
        if(writes_to_backbuffer) {
            rhi::RhiResourceBarrier backbuffer_barrier{};
            backbuffer_barrier.resource_to_barrier = ctx.swapchain_image;
            backbuffer_barrier.access_before_barrier = rhi::ResourceAccess::ColorAttachmentWrite;
            backbuffer_barrier.access_after_barrier = rhi::ResourceAccess::MemoryRead;
            backbuffer_barrier.old_state = rhi::ResourceState::RenderTarget;
            backbuffer_barrier.new_state = rhi::ResourceState::PresentSource;
            backbuffer_barrier.source_queue = rhi::QueueType::Graphics;
            backbuffer_barrier.destination_queue = rhi::QueueType::Graphics;
            backbuffer_barrier.image_memory_barrier.aspect = rhi::ImageAspect::Color;

            rx::vector<rhi::RhiResourceBarrier> barriers{&rx::memory::g_system_allocator};
            barriers.push_back(backbuffer_barrier);
            cmds.resource_barriers(rhi::PipelineStage::ColorAttachmentOutput, rhi::PipelineStage::BottomOfPipe, barriers);
        }
    }

    void SceneRenderpass::record_renderpass_contents(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) {}

    GlobalRenderpass::GlobalRenderpass(const rx::string& name, RhiGraphicsPipelineState pipeline_state, const bool is_builtin)
        : Renderpass{name, is_builtin}, pipeline_state{rx::utility::move(pipeline_state)} {}

    void GlobalRenderpass::record_renderpass_contents(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) {
        cmds.set_pipeline(pipeline_state);

        cmds.bind_resources(*resource_binder);

        // TODO: Bind a fullscreen triangle mesh

        cmds.draw_indexed_mesh(3);
    }

    Rendergraph::Rendergraph(rx::memory::allocator& allocator, rhi::RenderDevice& device) : allocator(allocator), device(device) {}

    void Rendergraph::destroy_renderpass(const rx::string& name) {
        if(Renderpass** renderpass = renderpasses.find(name)) {
            if((*renderpass)->framebuffer) {
                device.destroy_framebuffer((*renderpass)->framebuffer, allocator);
            }

            device.destroy_renderpass((*renderpass)->renderpass, allocator);

            renderpasses.erase(name);
            renderpass_metadatas.erase(name);

            is_dirty = true;
        }
    }

    rx::vector<rx::string> Rendergraph::calculate_renderpass_execution_order() {
        MTR_SCOPE("Rendergraph", "calculate_renderpass_execution_order");
        if(is_dirty) {
            const auto create_infos = [&]() {
                rx::vector<RenderPassCreateInfo> create_info_temp{&allocator};
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
                .on_error([&](const auto& err) { rg_log->error("Could not determine renderpass execution order: %s", err.to_string()); });

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

    rhi::RhiFramebuffer* Renderpass::get_framebuffer(const FrameContext& ctx) const {
        if(!writes_to_backbuffer) {
            return framebuffer;
        } else {
            return ctx.swapchain_framebuffer;
        }
    }

    void Renderpass::setup_renderpass(rhi::RhiRenderCommandList& /* cmds */, FrameContext& /* ctx */) {}

    void renderer::MaterialPass::record(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) const {
        MTR_SCOPE("MaterialPass", "record");

        cmds.bind_descriptor_sets(descriptor_sets, pipeline_interface);

        static_mesh_draws.each_fwd(
            [&](const MeshBatch<StaticMeshRenderCommand>& batch) { record_rendering_static_mesh_batch(batch, cmds, ctx); });

        static_procedural_mesh_draws.each_fwd(
            [&](const ProceduralMeshBatch<StaticMeshRenderCommand>& batch) { record_rendering_static_mesh_batch(batch, cmds, ctx); });
    }

    void renderer::MaterialPass::record_rendering_static_mesh_batch(const MeshBatch<StaticMeshRenderCommand>& batch,
                                                                    rhi::RhiRenderCommandList& cmds,
                                                                    FrameContext& ctx) {
        MTR_SCOPE("MaterialPass", "record_rendering_static_mesh_batch");
        const uint64_t start_index = ctx.cur_model_matrix_index;

        auto model_matrix_buffer = ctx.nova->get_resource_manager().get_uniform_buffer(MODEL_MATRIX_BUFFER_NAME);

        batch.commands.each_fwd([&](const StaticMeshRenderCommand& command) {
            if(command.is_visible) {
                /* ctx.nova->get_device().write_data_to_buffer(&command.model_matrix,
                                                             sizeof(glm::mat4),
                                                             ctx.cur_model_matrix_index * sizeof(glm::mat4),
                                                             (*model_matrix_buffer)->buffer);*/
                ctx.cur_model_matrix_index++;
            }
        });

        if(start_index != ctx.cur_model_matrix_index) {
            // TODO: There's probably a better way to do this
            rx::vector<rhi::RhiBuffer*> vertex_buffers{ctx.allocator};
            vertex_buffers.reserve(batch.num_vertex_attributes);
            for(uint32_t i = 0; i < batch.num_vertex_attributes; i++) {
                vertex_buffers.push_back(batch.vertex_buffer);
            }
            cmds.bind_vertex_buffers(vertex_buffers);
            cmds.bind_index_buffer(batch.index_buffer, rhi::IndexType::Uint32);

            cmds.draw_indexed_mesh(batch.num_indices);
        }
    }

    void renderer::MaterialPass::record_rendering_static_mesh_batch(const ProceduralMeshBatch<StaticMeshRenderCommand>& batch,
                                                                    rhi::RhiRenderCommandList& cmds,
                                                                    FrameContext& ctx) {
        MTR_SCOPE("MaterialPass", "record_rendering_static_mesh_batch (ProceduralMesh)");
        const uint64_t start_index = ctx.cur_model_matrix_index;

        auto model_matrix_buffer = ctx.nova->get_resource_manager().get_uniform_buffer(MODEL_MATRIX_BUFFER_NAME);

        batch.commands.each_fwd([&](const StaticMeshRenderCommand& command) {
            if(command.is_visible) {
                /*ctx.nova->get_device().write_data_to_buffer(&command.model_matrix,
                                                            sizeof(glm::mat4),
                                                            ctx.cur_model_matrix_index * sizeof(glm::mat4),
                                                            (*model_matrix_buffer)->buffer);*/
                ctx.cur_model_matrix_index++;
            }
        });

        if(start_index != ctx.cur_model_matrix_index) {
            const auto& [vertex_buffer, index_buffer] = batch.mesh->get_buffers_for_frame(ctx.frame_idx);
            // TODO: There's probably a better way to do this
            rx::vector<rhi::RhiBuffer*> vertex_buffers;
            vertex_buffers.reserve(7);
            for(uint32_t i = 0; i < 7; i++) {
                vertex_buffers.push_back(vertex_buffer);
            }
            cmds.bind_vertex_buffers(vertex_buffers);
            cmds.bind_index_buffer(index_buffer, rhi::IndexType::Uint32);
        }
    }

    void Pipeline::record(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) const {
        MTR_SCOPE("Pipeline", "record");
        cmds.set_pipeline(pipeline);

        const auto& passes = ctx.nova->get_material_passes_for_pipeline(pipeline.name);

        passes.each_fwd([&](const renderer::MaterialPass& pass) { pass.record(cmds, ctx); });
    }
} // namespace nova::renderer
