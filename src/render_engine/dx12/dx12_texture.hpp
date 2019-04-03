/*!
 * \author ddubois
 * \date 20-Oct-18.
 */

#ifndef NOVA_RENDERER_DX_12_TEXTURE_HPP
#define NOVA_RENDERER_DX_12_TEXTURE_HPP

#include <d3d12.h>
#include <nova_renderer/shaderpack_data.hpp>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace nova::renderer {
    /*!
     * \brief
     */
    class dx12_texture {
    public:
        dx12_texture() = default;
        explicit dx12_texture(const texture_create_into_t& data);

        dx12_texture(const texture_create_into_t& data, const D3D12_RESOURCE_DESC texture_desc);

        dx12_texture(const texture_create_into_t& data, ComPtr<ID3D12Resource> resource);

        const texture_create_into_t& get_data() const;
        ComPtr<ID3D12Resource> get_dx12_texture() const;
        bool is_depth_texture() const;
        DXGI_FORMAT get_dxgi_format() const;

    private:
        texture_create_into_t data;
        ComPtr<ID3D12Resource> resource;
    };
} // namespace nova::renderer

#endif // NOVA_RENDERER_DX_12_TEXTURE_HPP
