#pragma once

#include <d3d12.h>

#include <nova_renderer/shaderpack_data.hpp>
#include "nova_renderer/command_list.hpp"

#include "../../util/logger.hpp"

namespace nova::renderer::rhi {
    D3D12_RESOURCE_STATES to_dx12_state(ResourceState state);

    DXGI_FORMAT to_dxgi_format(shaderpack::PixelFormatEnum pixel_format);

    enum D3D12_BLEND to_dx12_blend(shaderpack::BlendFactorEnum blend_factor);

    D3D12_COMPARISON_FUNC to_dx12_compare_func(shaderpack::CompareOpEnum depth_func);

    D3D12_STENCIL_OP to_dx12_stencil_op(shaderpack::StencilOpEnum op);

    D3D12_PRIMITIVE_TOPOLOGY_TYPE to_dx12_topology(shaderpack::PrimitiveTopologyEnum primitive_mode);

    D3D12_DESCRIPTOR_RANGE_TYPE to_dx12_range_type(DescriptorType type);

#ifndef NDEBUG
#define CHECK_ERROR(expr, msg)                                                                                                             \
    {                                                                                                                                      \
        HRESULT hr = expr;                                                                                                                 \
        if(FAILED(hr)) {                                                                                                                   \
            NOVA_LOG(ERROR) << __FILE__ << "(" << __LINE__ << ") failed: " << msg;                                                         \
        }                                                                                                                                  \
    }
#else
#define CHECK_ERROR(expr, msg) expr;
#endif
} // namespace nova::renderer
