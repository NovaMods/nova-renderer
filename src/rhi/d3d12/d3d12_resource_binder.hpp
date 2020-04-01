#pragma once

#include <d3d12.h>
#include <rx/core/map.h>
#include <wrl/client.h>

#include "d3d12_structs.hpp"
#include "nova_renderer/rhi/resource_binder.hpp"

namespace nova::renderer::rhi {
    struct D3D12Pipeline;

    class D3D12ResourceBinder : public RhiResourceBinder {
    public:
        D3D12ResourceBinder(rx::memory::allocator& allocator,
                            Microsoft::WRL::ComPtr<ID3D12Device> device_in,
                            Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_in,
                            rx::map<rx::string, D3D12_CPU_DESCRIPTOR_HANDLE>& descriptors_in);

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

        [[nodiscard]] ID3D12RootSignature* get_root_signature() const;

    private:
        rx::map<rx::string, D3D12Image*> bound_images;
        rx::map<rx::string, D3D12Buffer*> bound_buffers;
        rx::map<rx::string, D3D12Sampler*> bound_samplers;

        Microsoft::WRL::ComPtr<ID3D12Device> device;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;
        rx::map<rx::string, D3D12_CPU_DESCRIPTOR_HANDLE>& descriptors;
    };
} // namespace nova::renderer::rhi
