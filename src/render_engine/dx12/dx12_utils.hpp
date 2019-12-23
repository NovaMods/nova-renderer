#pragma once

#pragma warning(push, 0)
#include <d3d12.h>
#pragma warning(pop)

#include "nova_renderer/rhi/command_list.hpp"
#include "nova_renderer/shaderpack_data.hpp"

namespace nova::renderer::rhi {
    [[nodiscard]] D3D12_RESOURCE_STATES to_dx12_state(ResourceState state);

    [[nodiscard]] DXGI_FORMAT to_dxgi_format(shaderpack::PixelFormatEnum pixel_format);

    [[nodiscard]] enum D3D12_BLEND to_dx12_blend(shaderpack::BlendFactorEnum blend_factor);

    [[nodiscard]] D3D12_COMPARISON_FUNC to_dx12_compare_func(shaderpack::CompareOpEnum depth_func);

    [[nodiscard]] D3D12_STENCIL_OP to_dx12_stencil_op(shaderpack::StencilOpEnum op);

    [[nodiscard]] D3D12_PRIMITIVE_TOPOLOGY_TYPE to_dx12_topology(shaderpack::PrimitiveTopologyEnum primitive_mode);

    [[nodiscard]] D3D12_DESCRIPTOR_RANGE_TYPE to_dx12_range_type(DescriptorType type);

    [[nodiscard]] std::pmr::vector<D3D12_INPUT_ELEMENT_DESC> get_input_descriptions();

    [[nodiscard]] std::string to_string(HRESULT hr);

#ifdef NOVA_DEBUG
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
} // namespace nova::renderer::rhi
