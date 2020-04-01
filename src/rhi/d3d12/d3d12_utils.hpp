#pragma once

#include <d3d12.h>
#include <rx/core/string.h>

#include "nova_renderer/renderpack_data.hpp"
#include "nova_renderer/rhi/rhi_enums.hpp"

namespace nova::renderer::rhi {
    void set_object_name(ID3D12Object* object, const rx::string& name);

    D3D12_FILTER to_d3d12_filter(TextureFilter filter, TextureFilter mag_filter);

    D3D12_TEXTURE_ADDRESS_MODE to_d3d12_address_mode(TextureCoordWrapMode wrap_mode);

    DXGI_FORMAT to_dxgi_format(PixelFormat format);
}
