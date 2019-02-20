/*!
 * \author ddubois
 * \date 20-Oct-18.
 */

#include "dx12_utils.hpp"

namespace nova::renderer {
    D3D12_BLEND to_dx12_blend(const blend_factor_enum blend_factor) {
        switch(blend_factor) {
            case blend_factor_enum::One:
                return D3D12_BLEND_ONE;

            case blend_factor_enum::Zero:
                return D3D12_BLEND_ZERO;

            case blend_factor_enum::SrcColor:
                return D3D12_BLEND_SRC_COLOR;

            case blend_factor_enum::DstColor:
                return D3D12_BLEND_DEST_COLOR;

            case blend_factor_enum::OneMinusSrcColor:
                return D3D12_BLEND_INV_SRC_COLOR;

            case blend_factor_enum::OneMinusDstColor:
                return D3D12_BLEND_INV_DEST_COLOR;

            case blend_factor_enum::SrcAlpha:
                return D3D12_BLEND_SRC_ALPHA;

            case blend_factor_enum::DstAlpha:
                return D3D12_BLEND_DEST_ALPHA;

            case blend_factor_enum::OneMinusSrcAlpha:
                return D3D12_BLEND_INV_SRC_ALPHA;

            case blend_factor_enum::OneMinusDstAlpha:
                return D3D12_BLEND_INV_DEST_ALPHA;

            default:
                return D3D12_BLEND_ONE;
        }
    }

    D3D12_COMPARISON_FUNC to_dx12_compare_func(const compare_op_enum depth_func) {
        switch(depth_func) {
            case compare_op_enum::Never:
                return D3D12_COMPARISON_FUNC_NEVER;

            case compare_op_enum::Less:
                return D3D12_COMPARISON_FUNC_LESS;

            case compare_op_enum::LessEqual:
                return D3D12_COMPARISON_FUNC_LESS_EQUAL;

            case compare_op_enum::Greater:
                return D3D12_COMPARISON_FUNC_GREATER;

            case compare_op_enum::GreaterEqual:
                return D3D12_COMPARISON_FUNC_GREATER_EQUAL;

            case compare_op_enum::Equal:
                return D3D12_COMPARISON_FUNC_EQUAL;

            case compare_op_enum::NotEqual:
                return D3D12_COMPARISON_FUNC_NOT_EQUAL;

            case compare_op_enum::Always:
                return D3D12_COMPARISON_FUNC_ALWAYS;

            default:
                return D3D12_COMPARISON_FUNC_ALWAYS;
        }
    }

    D3D12_STENCIL_OP to_dx12_stencil_op(const stencil_op_enum op) {
        switch(op) {
            case stencil_op_enum::Keep:
                return D3D12_STENCIL_OP_KEEP;
            case stencil_op_enum::Zero:
                return D3D12_STENCIL_OP_ZERO;
            case stencil_op_enum::Replace:
                return D3D12_STENCIL_OP_REPLACE;
            case stencil_op_enum::Incr:
                return D3D12_STENCIL_OP_INCR;
            case stencil_op_enum::IncrWrap:
                return D3D12_STENCIL_OP_INCR_SAT;
            case stencil_op_enum::Decr:
                return D3D12_STENCIL_OP_DECR;
            case stencil_op_enum::DecrWrap:
                return D3D12_STENCIL_OP_DECR_SAT;
            case stencil_op_enum::Invert:
                return D3D12_STENCIL_OP_INVERT;
            default:
                return D3D12_STENCIL_OP_KEEP;
        }
    }

    D3D12_PRIMITIVE_TOPOLOGY_TYPE to_dx12_topology(const primitive_topology_enum primitive_mode) {
        switch(primitive_mode) {
            case primitive_topology_enum::Triangles:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            case primitive_topology_enum::Lines:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

            default:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        }
    }

    enum DXGI_FORMAT to_dxgi_format(const pixel_format_enum pixel_format) {
        switch(pixel_format) {
            case pixel_format_enum::RGBA8:
                return DXGI_FORMAT_R8G8B8A8_UNORM;

            case pixel_format_enum::RGBA16F:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;

                return DXGI_FORMAT_R32G32B32_FLOAT;

            case pixel_format_enum::RGBA32F:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;

            case pixel_format_enum::Depth:
                return DXGI_FORMAT_D32_FLOAT;

            case pixel_format_enum::DepthStencil:
                return DXGI_FORMAT_D24_UNORM_S8_UINT;

            default:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
                ;
        }
    }

} // namespace nova