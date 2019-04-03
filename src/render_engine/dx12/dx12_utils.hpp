#pragma once

#include <d3d12.h>

#include <nova_renderer/shaderpack_data_t.hpp>
#include "nova_renderer/command_list.hpp"

namespace nova::renderer {
    D3D12_RESOURCE_STATES to_dx12_state(resource_state state);

    enum class pixel_format_enum;
    DXGI_FORMAT to_dxgi_format(pixel_format_enum pixel_format);

    enum D3D12_BLEND to_dx12_blend(blend_factor_enum blend_factor);

    D3D12_COMPARISON_FUNC to_dx12_compare_func(compare_op_enum depth_func);

    D3D12_STENCIL_OP to_dx12_stencil_op(stencil_op_enum op);

    D3D12_PRIMITIVE_TOPOLOGY_TYPE to_dx12_topology(primitive_topology_enum primitive_mode);
} // namespace nova::renderer
