#pragma once

#include <d3d12.h>
#include <rx/core/string.h>

#include "nova_renderer/renderpack_data.hpp"
#include "nova_renderer/rhi/rhi_enums.hpp"
#include "nova_renderer/rhi/pipeline_create_info.hpp"

namespace nova::renderer::rhi {
    void set_object_name(ID3D12Object* object, const rx::string& name);

    D3D12_FILTER to_d3d12_filter(TextureFilter filter, TextureFilter mag_filter);

    D3D12_TEXTURE_ADDRESS_MODE to_d3d12_address_mode(TextureCoordWrapMode wrap_mode);

    DXGI_FORMAT to_dxgi_format(PixelFormat format);

    D3D12_BLEND to_d3d12_blend(BlendFactor blend_factor);

    D3D12_BLEND_OP to_d3d12_blend_op(BlendOp blend_op);

    D3D12_FILL_MODE to_d3d12_fill_mode(FillMode fill_mode);

    D3D12_CULL_MODE to_d3d12_cull_mode(PrimitiveCullingMode cull_mode);

    D3D12_COMPARISON_FUNC to_d3d12_compare_func(CompareOp compare_op);

    D3D12_STENCIL_OP to_d3d12_stencil_op(StencilOp stencil_op);
}
