/*!
 * \author ddubois 
 * \date 20-Oct-18.
 */

#ifndef NOVA_RENDERER_DX_12_TEXTURE_HPP
#define NOVA_RENDERER_DX_12_TEXTURE_HPP

#include <d3d12.h>
#include <wrl.h>
#include "../../loading/shaderpack/shaderpack_data.hpp"

using Microsoft::WRL::ComPtr;

namespace nova {
    /*!
     * \brief
     */
    class dx12_texture {
    public:
        dx12_texture() = default;
        explicit dx12_texture(const texture_resource_data& data);

        dx12_texture(const texture_resource_data & data, const D3D12_RESOURCE_DESC texture_desc);

        dx12_texture(const texture_resource_data & data, ComPtr<ID3D12Resource> resource);

        const texture_resource_data& get_data() const;
        ComPtr<ID3D12Resource> get_dx12_texture() const;
        bool is_depth_texture() const;
        DXGI_FORMAT get_dxgi_format() const;
    private:
        texture_resource_data data;
        ComPtr<ID3D12Resource> resource;
    };
}

#endif //NOVA_RENDERER_DX_12_TEXTURE_HPP
