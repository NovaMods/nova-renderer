#pragma once

#include "nova_renderer/rhi/forward_decls.hpp"

#include "rendergraph.hpp"

namespace nova::renderer {
    struct FrameContext;

    class UiRenderpass : public Renderpass {
    public:
        UiRenderpass(rhi::RenderEngine* device, const glm::vec2& framebuffer_size);

        void render_renderpass_contents(rhi::CommandList* cmds, FrameContext& ctx) override;

    protected:
        /*!
         * \brief Renders the host application's UI
         *
         * Clients of Nova must provide their own implementation of `UiRenderpass`. Nova will then use that implementation to
         */
        virtual void render_ui(rhi::CommandList* cmds, FrameContext& ctx) = 0;
    };

    class NullUiRenderpass final : public UiRenderpass {
    public:
        void render(rhi::CommandList* cmds, FrameContext& ctx) override;

        ~NullUiRenderpass() override = default;

    protected:
        void render_ui(rhi::CommandList* cmds, FrameContext& ctx) override;
    };
} // namespace nova::renderer
