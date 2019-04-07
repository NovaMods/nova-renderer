/*!
 * \author ddubois
 * \date 20-Oct-18.
 */

#ifndef NOVA_RENDERER_DX_12_UTILS_HPP
#define NOVA_RENDERER_DX_12_UTILS_HPP

#include <d3d12.h>

#include <nova_renderer/shaderpack_data.hpp>
#include "nova_renderer/command_list.hpp"

#include "../../util/logger.hpp"

namespace nova::renderer::rhi {
    D3D12_RESOURCE_STATES to_dx12_state(resource_state state);

    DXGI_FORMAT to_dxgi_format(shaderpack::pixel_format_enum pixel_format);

    enum D3D12_BLEND to_dx12_blend(shaderpack::blend_factor_enum blend_factor);

    D3D12_COMPARISON_FUNC to_dx12_compare_func(shaderpack::compare_op_enum depth_func);

    D3D12_STENCIL_OP to_dx12_stencil_op(shaderpack::stencil_op_enum op);

    D3D12_PRIMITIVE_TOPOLOGY_TYPE to_dx12_topology(shaderpack::primitive_topology_enum primitive_mode);

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

#endif // NOVA_RENDERER_DX_12_UTILS_HPP
