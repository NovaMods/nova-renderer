#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "nova_renderer/shaderpack_data.hpp"

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
