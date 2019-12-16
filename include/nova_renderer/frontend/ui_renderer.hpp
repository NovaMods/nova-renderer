#pragma once

#include "nova_renderer/rhi/forward_decls.hpp"

#include "rendergraph.hpp"

namespace nova::renderer {
    struct FrameContext;

    class UiRenderpass : public Renderpass {
    public:
        void render_pass_contents(rhi::CommandList* cmds, FrameContext& ctx) override;

    protected:
        /*!
         * \brief Subclasses must override this method to provide UI rendering functionality
         */
        virtual void render_ui(rhi::CommandList* cmds, FrameContext& ctx) const = 0;
    };
} // namespace nova::renderer
