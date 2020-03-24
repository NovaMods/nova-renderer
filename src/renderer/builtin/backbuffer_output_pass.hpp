#pragma once
#include "nova_renderer/rendergraph.hpp"

namespace nova::renderer {
    class BackbufferOutputRenderpass final : public GlobalRenderpass {
    public:
        explicit BackbufferOutputRenderpass(rhi::RhiImage* ui_output,
                                            rhi::RhiImage* scene_output,
                                            rx::ptr<rhi::RhiPipeline> pipeline,
                                            rhi::RenderDevice& device);

        static const renderpack::RenderPassCreateInfo& get_create_info();

    protected:
        void record_post_renderpass_barriers(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) const override;

    private:
        rx::vector<rhi::RhiResourceBarrier> post_pass_barriers;
    };
} // namespace nova::renderer
