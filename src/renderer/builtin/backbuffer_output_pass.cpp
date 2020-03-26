#include "backbuffer_output_pass.hpp"

#include "nova_renderer/loading/renderpack_loading.hpp"
#include "nova_renderer/nova_renderer.hpp"

namespace nova::renderer {
    RX_LOG("BackbufferOut", logger);

    struct RX_HINT_EMPTY_BASES BackbufferOutputRenderpassCreateInfo : renderpack::RenderPassCreateInfo {
        BackbufferOutputRenderpassCreateInfo();
    };

    BackbufferOutputRenderpassCreateInfo::BackbufferOutputRenderpassCreateInfo() {
        name = BACKBUFFER_OUTPUT_RENDER_PASS_NAME;
        texture_inputs.reserve(2);
        texture_inputs.emplace_back(UI_OUTPUT_RT_NAME);
        texture_inputs.emplace_back(SCENE_OUTPUT_RT_NAME);

        texture_outputs.reserve(1);
        texture_outputs.emplace_back(BACKBUFFER_NAME, rhi::PixelFormat::Rgba8, false);

        pipeline_names.reserve(1);
        pipeline_names.emplace_back(BACKBUFFER_OUTPUT_PIPELINE_NAME);
    }

    rx::global<BackbufferOutputRenderpassCreateInfo> backbuffer_output_create_info{"Nova", "BackbufferOutputCreateInfo"};

    BackbufferOutputRenderpass::BackbufferOutputRenderpass(rhi::RhiImage* ui_output,
                                                           rhi::RhiImage* scene_output,
                                                           rhi::RhiSampler* point_sampler,
                                                           rx::ptr<rhi::RhiPipeline> pipeline,
                                                           MeshId mesh,
                                                           rhi::RenderDevice& device)
        : GlobalRenderpass(BACKBUFFER_OUTPUT_RENDER_PASS_NAME, rx::utility::move(pipeline), mesh, true) {
        resource_binder = device.create_resource_binder_for_pipeline(*(this->pipeline), device.get_allocator());

        resource_binder->bind_image("ui_output", ui_output);
        resource_binder->bind_image("scene_output", scene_output);
        resource_binder->bind_sampler("tex_sampler", point_sampler);

        rhi::RhiResourceBarrier pre_pass_barrier;
        pre_pass_barrier.access_before_barrier = rhi::ResourceAccess::ColorAttachmentWrite;
        pre_pass_barrier.access_after_barrier = rhi::ResourceAccess::ShaderRead;
        pre_pass_barrier.old_state = rhi::ResourceState::RenderTarget;
        pre_pass_barrier.new_state = rhi::ResourceState::ShaderRead;
        pre_pass_barrier.source_queue = rhi::QueueType::Graphics;
        pre_pass_barrier.destination_queue = rhi::QueueType::Graphics;
        pre_pass_barrier.image_memory_barrier.aspect = rhi::ImageAspect::Color;

        read_texture_barriers.reserve(2);

        pre_pass_barrier.resource_to_barrier = ui_output;
        read_texture_barriers.push_back(pre_pass_barrier);

        pre_pass_barrier.resource_to_barrier = scene_output;
        read_texture_barriers.push_back(pre_pass_barrier);

        rhi::RhiResourceBarrier post_pass_barrier;
        post_pass_barrier.access_before_barrier = rhi::ResourceAccess::ShaderRead;
        post_pass_barrier.access_after_barrier = rhi::ResourceAccess::ColorAttachmentWrite;
        post_pass_barrier.old_state = rhi::ResourceState::ShaderRead;
        post_pass_barrier.new_state = rhi::ResourceState::RenderTarget;
        post_pass_barrier.source_queue = rhi::QueueType::Graphics;
        post_pass_barrier.destination_queue = rhi::QueueType::Graphics;
        post_pass_barrier.image_memory_barrier.aspect = rhi::ImageAspect::Color;

        post_pass_barriers.reserve(2);

        post_pass_barrier.resource_to_barrier = ui_output;
        post_pass_barriers.push_back(post_pass_barrier);

        post_pass_barrier.resource_to_barrier = scene_output;
        post_pass_barriers.push_back(post_pass_barrier);
    }

    const renderpack::RenderPassCreateInfo& BackbufferOutputRenderpass::get_create_info() { return *backbuffer_output_create_info; }

    void BackbufferOutputRenderpass::record_post_renderpass_barriers(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) const {
        Renderpass::record_post_renderpass_barriers(cmds, ctx);

        // TODO: Figure out how to make the backend deal with the barriers
        cmds.resource_barriers(rhi::PipelineStage::FragmentShader, rhi::PipelineStage::ColorAttachmentOutput, post_pass_barriers);
    }
} // namespace nova::renderer
