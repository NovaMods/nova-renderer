#pragma once
#include "nova_renderer/rendergraph.hpp"

namespace nova::renderer {
    class BackbufferOutputRenderpass final : public Renderpass {
    public:
        explicit BackbufferOutputRenderpass(NovaRenderer* nova);

        static const shaderpack::RenderPassCreateInfo& get_create_info();
    };
} // namespace nova::renderer
