#pragma once
#include "nova_renderer/rendergraph.hpp"

namespace nova::renderer {
    class BackbufferOutputRenderpass final : public Renderpass {
    public:
        explicit BackbufferOutputRenderpass(rhi::RhiResource* ui_output, rhi::RhiResource* scene_output);

        static const renderpack::RenderPassCreateInfo& get_create_info();

    protected:
        void record_post_renderpass_barriers(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) const override;

    private:
        rx::vector<rhi::RhiResourceBarrier> post_pass_barriers;
    };
} // namespace nova::renderer
