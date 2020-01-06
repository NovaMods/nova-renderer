#include "nova_renderer/frontend/rendergraph.hpp"

#include "nova_renderer/nova_renderer.hpp"
#include "nova_renderer/rhi/command_list.hpp"
#include "nova_renderer/rhi/render_engine.hpp"
#include "nova_renderer/shaderpack_data.hpp"
#include "nova_renderer/util/logger.hpp"

namespace nova::renderer {
    void Renderpass::render(rhi::CommandList* cmds, FrameContext& ctx) {
        // TODO: Figure if any of these barriers are implicit
        // TODO: Use shader reflection to figure our the stage that the pipelines in this renderpass need access to this resource instead of
        // using a robust default

        record_pre_renderpass_barriers(cmds, ctx);

        const auto framebuffer = get_framebuffer(ctx);

        cmds->begin_renderpass(renderpass, framebuffer);

        render_renderpass_contents(cmds, ctx);

        cmds->end_renderpass();

        record_post_renderpass_barriers(cmds, ctx);
    }

    void Renderpass::record_pre_renderpass_barriers(rhi::CommandList* cmds, FrameContext& ctx) const {
        if(!read_texture_barriers.empty()) {
            // TODO: Use shader reflection to figure our the stage that the pipelines in this renderpass need access to this resource
            // instead of using a robust default
            cmds->resource_barriers(rhi::PipelineStage::ColorAttachmentOutput, rhi::PipelineStage::FragmentShader, read_texture_barriers);
        }

        if(!write_texture_barriers.empty()) {
            // TODO: Use shader reflection to figure our the stage that the pipelines in this renderpass need access to this resource
            // instead of using a robust default
            cmds->resource_barriers(rhi::PipelineStage::ColorAttachmentOutput, rhi::PipelineStage::FragmentShader, write_texture_barriers);
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
            cmds->resource_barriers(rhi::PipelineStage::TopOfPipe, rhi::PipelineStage::ColorAttachmentOutput, {backbuffer_barrier});
        }
    }

    void Renderpass::render_renderpass_contents(rhi::CommandList* cmds, FrameContext& ctx) {
        const auto pipeline_storage = ctx.nova->get_pipeline_storage();

        // TODO: I _actually_ want to get all the draw commands from NovaRenderer, instead of storing them in this struct
        for(const std::string& pipeline_name : pipeline_names) {
            const auto pipeline = pipeline_storage->get_pipeline(pipeline_name);
            if(pipeline) {
                pipeline->record(cmds, ctx);
            }
        }
    }

    void Renderpass::record_post_renderpass_barriers(rhi::CommandList* cmds, FrameContext& ctx) const {
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

            // When this line executes, the D3D12 debug layer gets mad about "A single command list cannot write to multiple buffers within
            // a particular swapchain" and I don't know why it's mad about that, or even really what that message means
            cmds->resource_barriers(rhi::PipelineStage::ColorAttachmentOutput, rhi::PipelineStage::BottomOfPipe, {backbuffer_barrier});
        }
    }

    rhi::Framebuffer* Renderpass::get_framebuffer(const FrameContext& ctx) const {
        if(!writes_to_backbuffer) {
            return framebuffer;
        } else {
            return ctx.swapchain_framebuffer;
        }
    }

    void MaterialPass::record(rhi::CommandList* cmds, FrameContext& ctx) const {
        cmds->bind_descriptor_sets(descriptor_sets, pipeline_interface);

        for(const MeshBatch<StaticMeshRenderCommand>& batch : static_mesh_draws) {
            record_rendering_static_mesh_batch(batch, cmds, ctx);
        }

        for(const ProceduralMeshBatch<StaticMeshRenderCommand>& batch : static_procedural_mesh_draws) {
            record_rendering_static_mesh_batch(batch, cmds, ctx);
        }
    }

    void MaterialPass::record_rendering_static_mesh_batch(const MeshBatch<StaticMeshRenderCommand>& batch,
                                                          rhi::CommandList* cmds,
                                                          FrameContext& ctx) {
        const uint64_t start_index = ctx.cur_model_matrix_index;

        for(const StaticMeshRenderCommand& command : batch.commands) {
            if(command.is_visible) {
                auto* model_matrix_buffer = ctx.nova->get_builtin_buffer(MODEL_MATRIX_BUFFER_NAME);
                ctx.nova->get_engine()->write_data_to_buffer(&command.model_matrix,
                                                             sizeof(glm::mat4),
                                                             ctx.cur_model_matrix_index * sizeof(glm::mat4),
                                                             model_matrix_buffer);
                ctx.cur_model_matrix_index++;
            }
        }

        if(start_index != ctx.cur_model_matrix_index) {
            // TODO: There's probably a better way to do this
            const std::pmr::vector<rhi::Buffer*> vertex_buffers = std::pmr::vector<rhi::Buffer*>(7, batch.vertex_buffer);
            cmds->bind_vertex_buffers(vertex_buffers);
            cmds->bind_index_buffer(batch.index_buffer);

            cmds->draw_indexed_mesh(static_cast<uint32_t>(batch.index_buffer->size.b_count()) / sizeof(uint32_t),
                                    static_cast<uint32_t>(ctx.cur_model_matrix_index - start_index));
        }
    }

    void MaterialPass::record_rendering_static_mesh_batch(const ProceduralMeshBatch<StaticMeshRenderCommand>& batch,
                                                          rhi::CommandList* cmds,
                                                          FrameContext& ctx) {
        const uint64_t start_index = ctx.cur_model_matrix_index;

        for(const StaticMeshRenderCommand& command : batch.commands) {
            if(command.is_visible) {
                auto* model_matrix_buffer = ctx.nova->get_builtin_buffer(MODEL_MATRIX_BUFFER_NAME);
                ctx.nova->get_engine()->write_data_to_buffer(&command.model_matrix,
                                                             sizeof(glm::mat4),
                                                             ctx.cur_model_matrix_index * sizeof(glm::mat4),
                                                             model_matrix_buffer);
                ctx.cur_model_matrix_index++;
            }
        }

        if(start_index != ctx.cur_model_matrix_index) {
            const auto& [vertex_buffer, index_buffer] = batch.mesh->get_buffers_for_frame(ctx.frame_count % NUM_IN_FLIGHT_FRAMES);
            // TODO: There's probably a better way to do this
            const std::pmr::vector<rhi::Buffer*> vertex_buffers(7, vertex_buffer);
            cmds->bind_vertex_buffers(vertex_buffers);
            cmds->bind_index_buffer(index_buffer);
        }
    }

    void Pipeline::record(rhi::CommandList* cmds, FrameContext& ctx) const {
        cmds->bind_pipeline(pipeline);

        const auto& passes = ctx.nova->get_material_passes_for_pipeline(pipeline);

        for(const MaterialPass& pass : passes) {
            pass.record(cmds, ctx);
        }
    }
} // namespace nova::renderer
