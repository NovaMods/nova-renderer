/*!
 * \author ddubois
 * \date 20-Oct-18.
 */

#include "dx12_utils.hpp"

namespace nova::renderer {
    D3D12_RESOURCE_STATES to_dx12_state(rhi::resource_state state) {
        switch(state) {
            case rhi::resource_state::UNDEFINED:
            case rhi::resource_state::GENERAL:
                return D3D12_RESOURCE_STATE_COMMON;

            case rhi::resource_state::COLOR_ATTACHMENT:
                return D3D12_RESOURCE_STATE_RENDER_TARGET;

            case rhi::resource_state::DEPTH_STENCIL_ATTACHMENT:
            case rhi::resource_state::DEPTH_ATTACHMENT_STENCIL_READ_ONLY:
                return D3D12_RESOURCE_STATE_DEPTH_WRITE;

            case rhi::resource_state::DEPTH_READ_ONLY_STENCIL_ATTACHMENT:
            case rhi::resource_state::DEPTH_STENCIL_READ_ONLY_ATTACHMENT:
                return D3D12_RESOURCE_STATE_DEPTH_READ;

            case rhi::resource_state::PRESENT_SOURCE:
                return D3D12_RESOURCE_STATE_PRESENT;

            case rhi::resource_state::NON_FRAGMENT_SHADER_READ_ONLY:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

            case rhi::resource_state::FRAGMENT_SHADER_READ_ONLY:
                return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

            case rhi::resource_state::TRANSFER_SOURCE:
                return D3D12_RESOURCE_STATE_COPY_SOURCE;

            case rhi::resource_state::TRANSFER_DESTINATION:
                return D3D12_RESOURCE_STATE_COPY_DEST;

            default:
                return D3D12_RESOURCE_STATE_COMMON;
        }
    }

    D3D12_BLEND to_dx12_blend(const shaderpack::blend_factor_enum blend_factor) {
        switch(blend_factor) {
            case shaderpack::blend_factor_enum::One:
                return D3D12_BLEND_ONE;

            case shaderpack::blend_factor_enum::Zero:
                return D3D12_BLEND_ZERO;

            case shaderpack::blend_factor_enum::SrcColor:
                return D3D12_BLEND_SRC_COLOR;

            case shaderpack::blend_factor_enum::DstColor:
                return D3D12_BLEND_DEST_COLOR;

            case shaderpack::blend_factor_enum::OneMinusSrcColor:
                return D3D12_BLEND_INV_SRC_COLOR;

            case shaderpack::blend_factor_enum::OneMinusDstColor:
                return D3D12_BLEND_INV_DEST_COLOR;

            case shaderpack::blend_factor_enum::SrcAlpha:
                return D3D12_BLEND_SRC_ALPHA;

            case shaderpack::blend_factor_enum::DstAlpha:
                return D3D12_BLEND_DEST_ALPHA;

            case shaderpack::blend_factor_enum::OneMinusSrcAlpha:
                return D3D12_BLEND_INV_SRC_ALPHA;

            case shaderpack::blend_factor_enum::OneMinusDstAlpha:
                return D3D12_BLEND_INV_DEST_ALPHA;

            default:
                return D3D12_BLEND_ONE;
        }
    }

    D3D12_COMPARISON_FUNC to_dx12_compare_func(const shaderpack::compare_op_enum depth_func) {
        switch(depth_func) {
            case shaderpack::compare_op_enum::Never:
                return D3D12_COMPARISON_FUNC_NEVER;

            case shaderpack::compare_op_enum::Less:
                return D3D12_COMPARISON_FUNC_LESS;

            case shaderpack::compare_op_enum::LessEqual:
                return D3D12_COMPARISON_FUNC_LESS_EQUAL;

            case shaderpack::compare_op_enum::Greater:
                return D3D12_COMPARISON_FUNC_GREATER;

            case shaderpack::compare_op_enum::GreaterEqual:
                return D3D12_COMPARISON_FUNC_GREATER_EQUAL;

            case shaderpack::compare_op_enum::Equal:
                return D3D12_COMPARISON_FUNC_EQUAL;

            case shaderpack::compare_op_enum::NotEqual:
                return D3D12_COMPARISON_FUNC_NOT_EQUAL;

            case shaderpack::compare_op_enum::Always:
                return D3D12_COMPARISON_FUNC_ALWAYS;

            default:
                return D3D12_COMPARISON_FUNC_ALWAYS;
        }
    }

    D3D12_STENCIL_OP to_dx12_stencil_op(const shaderpack::stencil_op_enum op) {
        switch(op) {
            case shaderpack::stencil_op_enum::Keep:
                return D3D12_STENCIL_OP_KEEP;
            case shaderpack::stencil_op_enum::Zero:
                return D3D12_STENCIL_OP_ZERO;
            case shaderpack::stencil_op_enum::Replace:
                return D3D12_STENCIL_OP_REPLACE;
            case shaderpack::stencil_op_enum::Incr:
                return D3D12_STENCIL_OP_INCR;
            case shaderpack::stencil_op_enum::IncrWrap:
                return D3D12_STENCIL_OP_INCR_SAT;
            case shaderpack::stencil_op_enum::Decr:
                return D3D12_STENCIL_OP_DECR;
            case shaderpack::stencil_op_enum::DecrWrap:
                return D3D12_STENCIL_OP_DECR_SAT;
            case shaderpack::stencil_op_enum::Invert:
                return D3D12_STENCIL_OP_INVERT;
            default:
                return D3D12_STENCIL_OP_KEEP;
        }
    }

    D3D12_PRIMITIVE_TOPOLOGY_TYPE to_dx12_topology(const shaderpack::primitive_topology_enum primitive_mode) {
        switch(primitive_mode) {
            case shaderpack::primitive_topology_enum::Triangles:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            case shaderpack::primitive_topology_enum::Lines:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

            default:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        }
    }

    enum DXGI_FORMAT to_dxgi_format(const shaderpack::pixel_format_enum pixel_format) {
        switch(pixel_format) {
            case shaderpack::pixel_format_enum::RGBA8:
                return DXGI_FORMAT_R8G8B8A8_UNORM;

            case shaderpack::pixel_format_enum::RGBA16F:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;

                return DXGI_FORMAT_R32G32B32_FLOAT;

            case shaderpack::pixel_format_enum::RGBA32F:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;

            case shaderpack::pixel_format_enum::Depth:
                return DXGI_FORMAT_D32_FLOAT;

            case shaderpack::pixel_format_enum::DepthStencil:
                return DXGI_FORMAT_D24_UNORM_S8_UINT;

            default:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
                ;
        }
    }

} // namespace nova::renderer