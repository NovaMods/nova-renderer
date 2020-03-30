#pragma once

#include <d3d12.h>

#include "nova_renderer/rhi/rhi_types.hpp"

namespace nova::renderer::rhi {
    struct D3D12RenderPass : RhiRenderpass {
        rx::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> render_target_descriptions;
        rx::optional<D3D12_RENDER_PASS_DEPTH_STENCIL_DESC> depth_stencil_description;

        D3D12_RENDER_PASS_FLAGS flags;
    };
} // namespace nova::renderer::rhi
