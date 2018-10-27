/*!
 * \author ddubois 
 * \date 20-Oct-18.
 */

#ifndef NOVA_RENDERER_DX_12_UTILS_HPP
#define NOVA_RENDERER_DX_12_UTILS_HPP

#include <d3d12.h>

#include "../../loading/shaderpack/shaderpack_data.hpp"

namespace nova {
    enum class pixel_format_enum;
    DXGI_FORMAT to_dxgi_format(pixel_format_enum pixel_format);

    enum D3D12_BLEND to_dx12_blend(blend_factor_enum blend_factor);

    D3D12_COMPARISON_FUNC to_dx12_compare_func(compare_op_enum depth_func);

    D3D12_STENCIL_OP to_dx12_stencil_op(stencil_op_enum op);

    D3D12_PRIMITIVE_TOPOLOGY_TYPE to_dx12_topology(primitive_topology_enum primitive_mode);
}


#endif //NOVA_RENDERER_DX_12_UTILS_HPP
