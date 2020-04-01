#include "d3d12_resource_binder.hpp"

#include <rx/core/log.h>

namespace nova::renderer::rhi {
    RX_LOG("D3D12ResourceBinder", logger);

    D3D12ResourceBinder::D3D12ResourceBinder(rx::memory::allocator& allocator,
                                             Microsoft::WRL::ComPtr<ID3D12Device> device_in,
                                             Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_in,
                                             rx::map<rx::string, D3D12_CPU_DESCRIPTOR_HANDLE>& descriptors_in)
        : bound_images{allocator},
          bound_buffers{allocator},
          bound_samplers{allocator},
          device{rx::utility::move(device_in)},
          root_signature{rx::utility::move(root_signature_in)},
          descriptors{descriptors_in} {}

    void D3D12ResourceBinder::bind_image(const rx::string& binding_name, RhiImage* image) {
        auto* d3d12_image = static_cast<D3D12Image*>(image);
        if(auto* image_slot = bound_images.find(binding_name)) {
            *image_slot = d3d12_image;

        } else {
            bound_images.insert(binding_name, d3d12_image);
        }
    }

    void D3D12ResourceBinder::bind_buffer(const rx::string& binding_name, RhiBuffer* buffer) {
        auto* d3d12_buffer = static_cast<D3D12Buffer*>(buffer);
        if(auto* buffer_slot = bound_buffers.find(binding_name)) {
            *buffer_slot = d3d12_buffer;

        } else {
            bound_buffers.insert(binding_name, d3d12_buffer);
        }
    }

    void D3D12ResourceBinder::bind_sampler(const rx::string& binding_name, RhiSampler* sampler) {
        auto* d3d12_sampler = static_cast<D3D12Sampler*>(sampler);
        if(auto* sampler_slot = bound_samplers.find(binding_name)) {
            *sampler_slot = d3d12_sampler;

        } else {
            bound_samplers.insert(binding_name, d3d12_sampler);
        }
    }

    void D3D12ResourceBinder::bind_image_array(const rx::string& /* binding_name */, const rx::vector<RhiImage*>& /* images */) {
        logger->error("D3D12 doesn't support binding image arrays");
    }

    void D3D12ResourceBinder::bind_buffer_array(const rx::string& /* binding_name */, const rx::vector<RhiBuffer*>& /* buffers */) {
        logger->error("D3D12 doesn't support binding buffer arrays");
    }

    void D3D12ResourceBinder::bind_sampler_array(const rx::string& /* binding_name */, const rx::vector<RhiSampler*>& /* samplers */) {
        logger->error("D3D12 doesn't support binding sampler arrays");
    }

    void D3D12ResourceBinder::update_descriptors() {
        bound_images.each_pair([&](const rx::string& binding_name, const D3D12Image* image) {
            const auto descriptor = *descriptors.find(binding_name);

            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
            srv_desc.Format = image->format;
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srv_desc.Texture2D.MipLevels = 1;

            device->CreateShaderResourceView(image->resource.Get(), &srv_desc, descriptor);
        });

        bound_buffers.each_pair([&](const rx::string& binding_name, const D3D12Buffer* buffer) {
            const auto descriptor = *descriptors.find(binding_name);

            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
            srv_desc.Format = DXGI_FORMAT_R8_UNORM;
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srv_desc.Buffer.NumElements = buffer->size.b_count();

            device->CreateShaderResourceView(buffer->resource.Get(), &srv_desc, descriptor);
        });

        bound_samplers.each_pair([&](const rx::string& binding_name, const D3D12Sampler* sampler) {
            const auto descriptor = *descriptors.find(binding_name);

            device->CreateSampler(&sampler->desc, descriptor);
        });
    }

    ID3D12RootSignature* D3D12ResourceBinder::get_root_signature() const {
        return root_signature.Get();
    }
} // namespace nova::renderer::rhi
