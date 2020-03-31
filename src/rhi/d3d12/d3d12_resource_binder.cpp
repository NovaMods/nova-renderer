#include "d3d12_resource_binder.hpp"

namespace nova::renderer::rhi {
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
        
    }
} // namespace nova::renderer::rhi
