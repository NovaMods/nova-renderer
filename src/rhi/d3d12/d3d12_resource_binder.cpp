#include "d3d12_resource_binder.hpp"

#include <rx/core/log.h>

#include "d3dx12.h"
#include "minitrace.h"

namespace nova::renderer::rhi {
    RX_LOG("D3D12ResourceBinder", logger);

    D3D12ResourceBinder::D3D12ResourceBinder(rx::memory::allocator& allocator,
                                             INT descriptor_size_in,
                                             Microsoft::WRL::ComPtr<ID3D12Device> device_in,
                                             Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_in,
                                             rx::vector<D3D12RootParameter> root_parameters_in,
                                             rx::map<rx::string, D3D12Descriptor> descriptor_table_bindings_in)
        : internal_allocator{&allocator},
          bound_images{allocator},
          bound_buffers{allocator},
          bound_samplers{allocator},
          device{rx::utility::move(device_in)},
          root_signature{rx::utility::move(root_signature_in)},
          root_parameters{rx::utility::move(root_parameters_in)},
          descriptor_table_bindings{rx::utility::move(descriptor_table_bindings_in)},
          descriptor_size{descriptor_size_in} {}

    void D3D12ResourceBinder::bind_image(const rx::string& binding_name, RhiImage* image) {
        bind_image_array(binding_name, rx::array{image});
    }

    void D3D12ResourceBinder::bind_buffer(const rx::string& binding_name, RhiBuffer* buffer) {
        bind_buffer_array(binding_name, rx::array{buffer});
    }

    void D3D12ResourceBinder::bind_sampler(const rx::string& /* binding_name */, RhiSampler* /* sampler */) {
        logger->error("D3D12 doesn't support binding samplers");
    }

    void D3D12ResourceBinder::bind_image_array(const rx::string& binding_name, const rx::vector<RhiImage*>& images) {
        rx::vector<D3D12Image*> d3d12_images{*internal_allocator};
        d3d12_images.reserve(images.size());

        images.each_fwd([&](RhiImage* image) { d3d12_images.push_back(static_cast<D3D12Image*>(image)); });

        if(auto* image_slot = bound_images.find(binding_name)) {
            *image_slot += d3d12_images;

        } else {
            bound_images.insert(binding_name, d3d12_images);
        }

        is_dirty = true;
    }

    void D3D12ResourceBinder::bind_buffer_array(const rx::string& binding_name, const rx::vector<RhiBuffer*>& buffers) {
        rx::vector<D3D12Buffer*> d3d12_buffers{*internal_allocator};
        d3d12_buffers.reserve(buffers.size());

        buffers.each_fwd([&](RhiBuffer* image) { d3d12_buffers.push_back(static_cast<D3D12Buffer*>(image)); });

        if(auto* buffer_slot = bound_buffers.find(binding_name)) {
            *buffer_slot += d3d12_buffers;

        } else {
            bound_buffers.insert(binding_name, d3d12_buffers);
        }

        is_dirty = true;
    }

    void D3D12ResourceBinder::bind_sampler_array(const rx::string& /* binding_name */, const rx::vector<RhiSampler*>& /* samplers */) {
        logger->error("D3D12 doesn't support binding sampler arrays");
    }

    void D3D12ResourceBinder::update_descriptors() {
        MTR_SCOPE("D3D12ResourceBinder", "update_descriptors");

        descriptor_table_bindings.each_pair([&](const rx::string& binding, const D3D12Descriptor& descriptor) {
            if(auto* images_slot = bound_images.find(binding)) {
                auto& images = *images_slot;

                switch(descriptor.type) {
                    case D3D12ResourceType::CBV: {
                        logger->error("Can not bind a texture to constant buffer binding %s", binding);
                    } break;

                    case D3D12ResourceType::SRV: {
                        create_srv_array(images, descriptor);
                    } break;

                    case D3D12ResourceType::UAV: {
                        create_uav_array(images, descriptor);
                    } break;
                }

            } else if(auto* buffers_slot = bound_buffers.find(binding)) {
                auto* buffers = *buffers_slot;

                switch(descriptor.type) {
                    case D3D12ResourceType::CBV: {
                        create_cbv_array(buffers, descriptor);
                    } break;

                    case D3D12ResourceType::SRV: {
                        create_srv_array(buffers, descriptor);
                    } break;

                    case D3D12ResourceType::UAV: {
                        create_uav_array(buffers, descriptor);
                    } break;
                }

            } else {
                logger->error("No resource bound to binding %s", binding);
            }
        });
    }

    void D3D12ResourceBinder::bind_descriptors_to_command_list(ID3D12GraphicsCommandList* cmds) const {
        MTR_SCOPE("D3D12ResourceBinder", "bind_descriptors_to_command_list");

        cmds->SetGraphicsRootSignature(root_signature.Get());

        for(uint32_t i = 0; i < root_parameters.size(); i++) {
            const auto& parameter = root_parameters[i];
            if(parameter.type == D3D12RootParameter::Type::RootDescriptor) {
                bind_root_descriptor(i, parameter.root_descriptor, cmds);

            } else if(parameter.type == D3D12RootParameter::Type::DescriptorTable) {
                cmds->SetGraphicsRootDescriptorTable(i, parameter.descriptor_table.descriptor_table_start);
            }
        }
    }

    void D3D12ResourceBinder::bind_root_descriptor(const uint32_t slot_idx,
                                                   const D3D12RootDescriptor& root_descriptor,
                                                   ID3D12GraphicsCommandList* cmds) const {
        if(const auto* image_slot = bound_images.find(root_descriptor.binding_name)) {
            const auto& images = *image_slot;
            if(images.size() != 1) {
                logger->error("Can not bind more than one resource to root binding %s", root_descriptor.binding_name);
                return;
            }

            const auto* image = images[0];

            switch(root_descriptor.type) {
                case D3D12ResourceType::CBV: {
                    logger->error("Can not bind an image to CBV descriptor %s", root_descriptor.binding_name);
                } break;

                case D3D12ResourceType::SRV: {
                    cmds->SetGraphicsRootShaderResourceView(slot_idx, image->resource->GetGPUVirtualAddress());
                } break;

                case D3D12ResourceType::UAV: {
                    cmds->SetGraphicsRootUnorderedAccessView(slot_idx, image->resource->GetGPUVirtualAddress());
                } break;
            }

        } else if(const auto* buffer_slot = bound_buffers.find(root_descriptor.binding_name)) {
            const auto& buffers = *buffer_slot;
            if(buffers.size() != 1) {
                logger->error("Can not bind more than one resource to root binding %s", root_descriptor.binding_name);
                return;
            }

            const auto* buffer = buffers[0];

            switch(root_descriptor.type) {
                case D3D12ResourceType::CBV: {
                    cmds->SetGraphicsRootConstantBufferView(slot_idx, buffer->resource->GetGPUVirtualAddress());
                } break;

                case D3D12ResourceType::SRV: {
                    cmds->SetGraphicsRootShaderResourceView(slot_idx, buffer->resource->GetGPUVirtualAddress());
                } break;

                case D3D12ResourceType::UAV: {
                    cmds->SetGraphicsRootUnorderedAccessView(slot_idx, buffer->resource->GetGPUVirtualAddress());
                } break;
            }

        } else {
            logger->error("No resources bound to binding %s", root_descriptor.binding_name);
        }
    }

    void D3D12ResourceBinder::create_srv_array(const rx::vector<D3D12Image*>& images, const D3D12Descriptor& start_descriptor) const {
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle{start_descriptor.handle};

        images.each_fwd([&](const D3D12Image* image) {
            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
            srv_desc.Format = image->format;
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srv_desc.Texture2D.MostDetailedMip = 0;
            srv_desc.Texture2D.MipLevels = -1;
            srv_desc.Texture2D.PlaneSlice = 0;
            srv_desc.Texture2D.ResourceMinLODClamp = 0;

            device->CreateShaderResourceView(image->resource.Get(), &srv_desc, handle);

            handle.Offset(descriptor_size);
        });
    }

    void D3D12ResourceBinder::create_uav_array(const rx::vector<D3D12Image*>& images, const D3D12Descriptor& start_descriptor) const {
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle{start_descriptor.handle};

        images.each_fwd([&](const D3D12Image* image) {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc;
            uav_desc.Format = image->format;
            uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            uav_desc.Texture2D.MipSlice = 0;
            uav_desc.Texture2D.PlaneSlice = 0;

            device->CreateUnorderedAccessView(image->resource.Get(), nullptr, &uav_desc, handle);

            handle.Offset(descriptor_size);
        });
    }

    void D3D12ResourceBinder::create_cbv_array(const rx::vector<D3D12Buffer*>& buffers, const D3D12Descriptor& start_descriptor) const {
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle{start_descriptor.handle};

        buffers.each_fwd([&](const D3D12Buffer* buffer) {
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc{};
            cbv_desc.BufferLocation = buffer->resource->GetGPUVirtualAddress();
            cbv_desc.SizeInBytes = static_cast<UINT>(buffer->size.b_count());

            device->CreateConstantBufferView(&cbv_desc, handle);

            handle.Offset(descriptor_size);
        });
    }

    void D3D12ResourceBinder::create_srv_array(const rx::vector<D3D12Buffer*>& buffers, const D3D12Descriptor& start_descriptor) const {
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle{start_descriptor.handle};

        buffers.each_fwd([&](const D3D12Buffer* buffer) {
            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
            srv_desc.Format = DXGI_FORMAT_UNKNOWN;
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srv_desc.Buffer.FirstElement = 0;
            srv_desc.Buffer.NumElements = buffer->size.b_count() / start_descriptor.array_element_size;
            srv_desc.Buffer.StructureByteStride = start_descriptor.array_element_size;

            device->CreateShaderResourceView(buffer->resource.Get(), &srv_desc, handle);

            handle.Offset(descriptor_size);
        });
    }

    void D3D12ResourceBinder::create_uav_array(const rx::vector<D3D12Buffer*>& buffers, const D3D12Descriptor& start_descriptor) const {
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle{start_descriptor.handle};

        buffers.each_fwd([&](const D3D12Buffer* buffer) {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
            uav_desc.Format = DXGI_FORMAT_UNKNOWN;
            uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uav_desc.Buffer.FirstElement = 0;
            uav_desc.Buffer.NumElements = buffer->size.b_count() / start_descriptor.array_element_size;
            uav_desc.Buffer.StructureByteStride = start_descriptor.array_element_size;

            device->CreateUnorderedAccessView(buffer->resource.Get(), nullptr, &uav_desc, handle);

            handle.Offset(descriptor_size);
        });
    }
} // namespace nova::renderer::rhi
