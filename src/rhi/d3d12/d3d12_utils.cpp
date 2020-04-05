#include "d3d12_utils.hpp"

#include <d3dcompiler.h>
#include <rx/core/log.h>
#include <wrl/client.h>

#include "nova_renderer/rhi/pipeline_create_info.hpp"

using Microsoft::WRL::ComPtr;

namespace nova::renderer::rhi {
    RX_LOG("D3D12Utils", logger);

    void set_object_name(ID3D12Object* object, const rx::string& name) {
        const auto wide_name = name.to_utf16();

        object->SetName(reinterpret_cast<LPCWSTR>(wide_name.data()));
    }

    D3D12_FILTER to_d3d12_filter(const TextureFilter min_filter, const TextureFilter mag_filter) {
        switch(min_filter) {
            case TextureFilter::Point: {
                switch(mag_filter) {
                    case TextureFilter::Point:
                        return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;

                    case TextureFilter::Bilinear:
                        return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;

                    case TextureFilter::Trilinear:
                        return D3D12_FILTER_COMPARISON_ANISOTROPIC;
                }
            } break;

            case TextureFilter::Bilinear: {
                switch(mag_filter) {
                    case TextureFilter::Point:
                        return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;

                    case TextureFilter::Bilinear:
                        return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;

                    case TextureFilter::Trilinear:
                        return D3D12_FILTER_COMPARISON_ANISOTROPIC;
                }
            } break;

            case TextureFilter::Trilinear:
                return D3D12_FILTER_COMPARISON_ANISOTROPIC;
        }

        // If nothing else works cause someone passed in an invalid enum value for some stupid reason, you get a linear filter and you get
        // to like it
        return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    }

    D3D12_TEXTURE_ADDRESS_MODE to_d3d12_address_mode(const TextureCoordWrapMode wrap_mode) {
        switch(wrap_mode) {
            case TextureCoordWrapMode::Repeat:
                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;

            case TextureCoordWrapMode::MirroredRepeat:
                return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;

            case TextureCoordWrapMode::ClampToEdge:
                return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

            case TextureCoordWrapMode::ClampToBorder:
                return D3D12_TEXTURE_ADDRESS_MODE_BORDER;

            case TextureCoordWrapMode::MirrorClampToEdge:
                return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;

            default:
                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
    }

    DXGI_FORMAT to_dxgi_format(const PixelFormat format) {
        switch(format) {
            case PixelFormat::Rgba16F:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;

            case PixelFormat::Rgba32F:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;

            case PixelFormat::Depth32:
                return DXGI_FORMAT_D32_FLOAT;

            case PixelFormat::Depth24Stencil8:
                return DXGI_FORMAT_D24_UNORM_S8_UINT;

            case PixelFormat::Rgba8:
                [[fallthrough]];
            default:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
        }
    }

    D3D12_BLEND to_d3d12_blend(const BlendFactor blend_factor) {
        switch(blend_factor) {
            case BlendFactor::Zero:
                return D3D12_BLEND_ZERO;

            case BlendFactor::One:
                return D3D12_BLEND_ONE;

            case BlendFactor::SrcColor:
                return D3D12_BLEND_SRC_COLOR;

            case BlendFactor::OneMinusSrcColor:
                return D3D12_BLEND_INV_SRC_COLOR;

            case BlendFactor::DstColor:
                return D3D12_BLEND_DEST_COLOR;

            case BlendFactor::OneMinusDstColor:
                return D3D12_BLEND_INV_DEST_COLOR;

            case BlendFactor::SrcAlpha:
                return D3D12_BLEND_SRC_ALPHA;

            case BlendFactor::OneMinusSrcAlpha:
                return D3D12_BLEND_INV_SRC_ALPHA;

            case BlendFactor::DstAlpha:
                return D3D12_BLEND_DEST_ALPHA;

            case BlendFactor::OneMinusDstAlpha:
                return D3D12_BLEND_INV_DEST_ALPHA;

            case BlendFactor::ConstantColor:
                return D3D12_BLEND_BLEND_FACTOR;

            case BlendFactor::OneMinusConstantColor:
                return D3D12_BLEND_INV_BLEND_FACTOR;

            case BlendFactor::ConstantAlpha:
                return D3D12_BLEND_BLEND_FACTOR;

            case BlendFactor::OneMinusConstantAlpha:
                return D3D12_BLEND_INV_BLEND_FACTOR;

            case BlendFactor::ClampedSrcAlpha:
                return D3D12_BLEND_SRC_ALPHA_SAT;

            default:
                return D3D12_BLEND_ONE;
        }
    }

    D3D12_BLEND_OP to_d3d12_blend_op(const BlendOp blend_op) {
        switch(blend_op) {
            case BlendOp::Add:
                return D3D12_BLEND_OP_ADD;

            case BlendOp::Subtract:
                return D3D12_BLEND_OP_SUBTRACT;

            case BlendOp::ReverseSubtract:
                return D3D12_BLEND_OP_REV_SUBTRACT;

            case BlendOp::Min:
                return D3D12_BLEND_OP_MIN;

            case BlendOp::Max:
                return D3D12_BLEND_OP_MAX;

            default:
                return D3D12_BLEND_OP_ADD;
        }
    }

    D3D12_FILL_MODE to_d3d12_fill_mode(const FillMode fill_mode) {
        switch(fill_mode) {
            case FillMode::Solid:
                return D3D12_FILL_MODE_SOLID;

            case FillMode::Wireframe:
                [[fallthrough]];
            case FillMode::Points:
                return D3D12_FILL_MODE_WIREFRAME;

            default:
                return D3D12_FILL_MODE_SOLID;
        }
    }

    D3D12_CULL_MODE to_d3d12_cull_mode(const PrimitiveCullingMode cull_mode) {
        switch(cull_mode) {
            case PrimitiveCullingMode::BackFace:
                return D3D12_CULL_MODE_BACK;

            case PrimitiveCullingMode::FrontFace:
                return D3D12_CULL_MODE_FRONT;

            case PrimitiveCullingMode::None:
                return D3D12_CULL_MODE_NONE;

            default:
                return D3D12_CULL_MODE_BACK;
        }
    }

    D3D12_COMPARISON_FUNC to_d3d12_compare_func(const CompareOp compare_op) {
        switch(compare_op) {
            case CompareOp::Never:
                return D3D12_COMPARISON_FUNC_NEVER;

            case CompareOp::Less:
                return D3D12_COMPARISON_FUNC_LESS;

            case CompareOp::LessEqual:
                return D3D12_COMPARISON_FUNC_LESS_EQUAL;

            case CompareOp::Greater:
                return D3D12_COMPARISON_FUNC_GREATER;

            case CompareOp::GreaterEqual:
                return D3D12_COMPARISON_FUNC_GREATER_EQUAL;

            case CompareOp::Equal:
                return D3D12_COMPARISON_FUNC_EQUAL;

            case CompareOp::NotEqual:
                return D3D12_COMPARISON_FUNC_NOT_EQUAL;

            case CompareOp::Always:
                [[fallthrough]];
            default:
                return D3D12_COMPARISON_FUNC_ALWAYS;
        }
    }

    D3D12_STENCIL_OP to_d3d12_stencil_op(const StencilOp stencil_op) {
        switch(stencil_op) {
            case StencilOp::Keep:
                return D3D12_STENCIL_OP_KEEP;

            case StencilOp::Zero:
                return D3D12_STENCIL_OP_ZERO;

            case StencilOp::Replace:
                return D3D12_STENCIL_OP_REPLACE;

            case StencilOp::Increment:
                return D3D12_STENCIL_OP_INCR;

            case StencilOp::IncrementAndWrap:
                return D3D12_STENCIL_OP_INCR_SAT;

            case StencilOp::Decrement:
                return D3D12_STENCIL_OP_DECR;

            case StencilOp::DecrementAndWrap:
                return D3D12_STENCIL_OP_DECR_SAT;

            case StencilOp::Invert:
                return D3D12_STENCIL_OP_INVERT;

            default:
                return D3D12_STENCIL_OP_KEEP;
        }
    }

    uint32_t size_in_bytes(const DXGI_FORMAT format) {
        // TODO: Fill in with more formats as needed
        switch(format) {
            case DXGI_FORMAT_R32_FLOAT:
                return 4;

            case DXGI_FORMAT_R32G32_FLOAT:
                return 8;

            case DXGI_FORMAT_R32G32B32_FLOAT:
                return 12;

            case DXGI_FORMAT_R32G32B32A32_FLOAT:
                return 16;

            default:
                // Will hopefully cause enough problems that I'll know
                return 0;
        }
    }
} // namespace nova::renderer::rhi
