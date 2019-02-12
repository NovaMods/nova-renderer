/*!
 * \author ddubois
 * \date 20-Oct-18.
 */

#include "dx12_texture.hpp"
#include "dx12_utils.hpp"

namespace nova {
    dx12_texture::dx12_texture(const texture_resource_data& data, ComPtr<ID3D12Resource> resource) : data(data), resource(resource) {
    }

    const texture_resource_data& dx12_texture::get_data() const {
        return data;
    }

    ComPtr<ID3D12Resource> dx12_texture::get_dx12_texture() const {
        return resource;
    }

    bool dx12_texture::is_depth_texture() const {
        return data.format.pixel_format == pixel_format_enum::Depth || data.format.pixel_format == pixel_format_enum::DepthStencil;
    }

    DXGI_FORMAT dx12_texture::get_dxgi_format() const {
        return to_dxgi_format(data.format.pixel_format);
    }
} // namespace nova
