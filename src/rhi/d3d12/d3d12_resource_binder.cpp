#include "d3d12_resource_binder.hpp"

#include <rx/core/log.h>

namespace nova::renderer::rhi {
    RX_LOG("D3D12ResourceBinder", logger);

    D3D12ResourceBinder::D3D12ResourceBinder(rx::memory::allocator& allocator,
                                             Microsoft::WRL::ComPtr<ID3D12Device> device_in,
                                             Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_in,
                                             rx::map<rx::string, UINT> root_descriptor_bindings_in,
                                             rx::map<rx::string, D3D12_CPU_DESCRIPTOR_HANDLE> descriptor_table_bindings_in,
                                             rx::vector<D3D12RootSignatureSlotType> slot_types_in)
        : bound_images{allocator},
          bound_buffers{allocator},
          bound_samplers{allocator},
          device{rx::utility::move(device_in)},
          root_signature{rx::utility::move(root_signature_in)},
          root_descriptor_bindings{rx::utility::move(root_descriptor_bindings_in)},
          descriptor_table_bindings{rx::utility::move(descriptor_table_bindings_in)},
          slot_types{rx::utility::move(slot_types_in)} {}

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
        // TODO: Update any descriptors in a descriptor table
    }

    void D3D12ResourceBinder::bind_descriptors_to_command_list(ID3D12GraphicsCommandList* cmds) const {
        cmds->SetGraphicsRootSignature(root_signature.Get());

        root_descriptor_bindings.each_pair([&](const rx::string& binding_name, const UINT binding_idx) {
            if(const auto* image = bound_images.find(binding_name)) {
                cmds->SetGraphicsRootShaderResourceView(binding_idx, (*image)->resource->GetGPUVirtualAddress());

            } else if(const auto* buffer = bound_buffers.find(binding_name)) {
                if((*buffer)->is_constant_buffer) {
                    cmds->SetGraphicsRootConstantBufferView(binding_idx, (*buffer)->resource->GetGPUVirtualAddress());

                } else {
                    cmds->SetGraphicsRootShaderResourceView(binding_idx, (*buffer)->resource->GetGPUVirtualAddress());
                }
            }
        });
    }
} // namespace nova::renderer::rhi
