#pragma once
#include "nova_renderer/rendergraph.hpp"

namespace nova::renderer {
    class BackbufferOutputRenderpass final : public Renderpass {
    public:
        explicit BackbufferOutputRenderpass(rhi::Resource* ui_output, rhi::Resource* scene_output);

        static const shaderpack::RenderPassCreateInfo& get_create_info();

    protected:
        void record_post_renderpass_barriers(rhi::CommandList& cmds, FrameContext& ctx) const override;

    private:
        rx::vector<rhi::ResourceBarrier> post_pass_barriers;
    };
} // namespace nova::renderer
