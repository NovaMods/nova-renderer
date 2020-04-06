#pragma once

#include <d3d12.h>
#include <rx/core/map.h>
#include <wrl/client.h>

#include "nova_renderer/rhi/resource_binder.hpp"

#include "d3d12_enums.hpp"
#include "d3d12_structs.hpp"

namespace nova::renderer::rhi {
    struct D3D12Pipeline;

    class D3D12ResourceBinder : public RhiResourceBinder {
    public:
        D3D12ResourceBinder(rx::memory::allocator& allocator,
                            Microsoft::WRL::ComPtr<ID3D12Device> device_in,
                            Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_in,
                            rx::map<rx::string, UINT> root_descriptor_bindings_in,
                            rx::map<rx::string, D3D12_CPU_DESCRIPTOR_HANDLE> descriptor_table_bindings_in,
                            rx::vector<D3D12RootSignatureSlotType> slot_types_in);

        ~D3D12ResourceBinder() override = default;

#pragma region RhiResourceBinder
        void bind_image(const rx::string& binding_name, RhiImage* image) override;

        void bind_buffer(const rx::string& binding_name, RhiBuffer* buffer) override;

        void bind_sampler(const rx::string& binding_name, RhiSampler* sampler) override;

        void bind_image_array(const rx::string& binding_name, const rx::vector<RhiImage*>& images) override;

        void bind_buffer_array(const rx::string& binding_name, const rx::vector<RhiBuffer*>& buffers) override;

        void bind_sampler_array(const rx::string& binding_name, const rx::vector<RhiSampler*>& samplers) override;
#pragma endregion

        void update_descriptors();

        void bind_descriptors_to_command_list(ID3D12GraphicsCommandList* cmds) const;

    private:
        rx::map<rx::string, D3D12Image*> bound_images;
        rx::map<rx::string, D3D12Buffer*> bound_buffers;
        rx::map<rx::string, D3D12Sampler*> bound_samplers;

        Microsoft::WRL::ComPtr<ID3D12Device> device;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;

        /*!
         * \brief Map that tells us which bindings are root descriptors
         */
        rx::map<rx::string, UINT> root_descriptor_bindings;

        /*!
         * \brief Map of all the bindings which are part of a descriptor table
         */
        rx::map<rx::string, D3D12_CPU_DESCRIPTOR_HANDLE> descriptor_table_bindings;

        rx::vector<D3D12RootSignatureSlotType> slot_types;
    };
} // namespace nova::renderer::rhi
