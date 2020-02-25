#pragma once

#include "nova_renderer/rhi/forward_decls.hpp"

#include "rendergraph.hpp"

namespace nova::renderer {
    struct FrameContext;

    class UiRenderpass : public Renderpass {
    public:
        UiRenderpass();

        UiRenderpass(UiRenderpass&& old) noexcept = default;
        UiRenderpass& operator=(UiRenderpass&& old) noexcept = default;

        static const renderpack::RenderPassCreateInfo& get_create_info();

    protected:
        void record_renderpass_contents(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) override final;

        /*!
         * \brief Renders the host application's UI
         *
         * Clients of Nova must provide their own implementation of `UiRenderpass`. Nova will then use that implementation to render that
         * application's UI
         */
        virtual void render_ui(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) = 0;
    };

    class NullUiRenderpass final : public UiRenderpass {
    public:
        ~NullUiRenderpass() override = default;

    protected:
        void render_ui(rhi::RhiRenderCommandList& cmds, FrameContext& ctx) override;
    };
} // namespace nova::renderer
