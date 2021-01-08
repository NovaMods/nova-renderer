#pragma once
#include "nova_renderer/rendergraph.hpp"

namespace nova::renderer {
    class BackbufferOutputRenderpass final : public GlobalRenderpass {
    public:
        explicit BackbufferOutputRenderpass(rhi::RhiImage* ui_output,
                                            rhi::RhiImage* scene_output,
                                            rhi::RhiSampler* point_sampler,
                                            rx::ptr<rhi::RhiPipeline> pipeline,
                                            MeshId mesh,
                                            rhi::RenderDevice& device);

        static const renderpack::RenderPassCreateInfo& get_create_info();

    protected:
        void record_post_renderpass_barriers(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) const override;

    private:
        std::vector<rhi::RhiResourceBarrier> post_pass_barriers;
    };
} // namespace nova::renderer
