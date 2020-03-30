#include "d3d12_render_device.hpp"

#include "nova_renderer/constants.hpp"

#include "rx/core/log.h"
using namespace Microsoft::WRL;

namespace nova::renderer ::rhi {
    RX_LOG("D3D12RenderDevice", logger);

    D3D12RenderDevice::D3D12RenderDevice(NovaSettingsAccessManager& settings, NovaWindow& window, rx::memory::allocator& allocator)
        : RenderDevice(settings, window, allocator) {
        initialize_dxgi();

        select_adapter();
    }

    void D3D12RenderDevice::initialize_dxgi() { CreateDXGIFactory(IID_PPV_ARGS(&factory)); }

    void D3D12RenderDevice::select_adapter() {
        // We want an adapter:
        // - Not integrated, if possible

        // TODO: Figure out how to get the number of adapters in advance
        rx::vector<ComPtr<IDXGIAdapter>> adapters{&internal_allocator};

        UINT adapter_idx = 0;
        ComPtr<IDXGIAdapter> cur_adapter;
        while(factory->EnumAdapters(adapter_idx, &cur_adapter) != DXGI_ERROR_NOT_FOUND) {
            adapters.push_back(cur_adapter);
            adapter_idx++;
        }

        // TODO: Score adapters based on things like supported feature level and available vram

        adapters.each_fwd([&](ComPtr<IDXGIAdapter> adapter) {
            DXGI_ADAPTER_DESC desc;
            adapter->GetDesc(&desc);

            if(desc.VendorId == INTEL_PCI_VENDOR_ID && adapters.size() > 1) {
                // Prefer something other then the Intel GPU
                return true;
            }

            ComPtr<ID3D12Device> try_device;
            auto res = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&try_device));
            if(SUCCEEDED(res)) {
                // check the features we care about
                D3D12_FEATURE_DATA_D3D12_OPTIONS d3d12_options;
                try_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &d3d12_options, sizeof(d3d12_options));
                if(d3d12_options.ResourceBindingTier != D3D12_RESOURCE_BINDING_TIER_3) {
                    // Resource binding tire three means we can have partially bound descriptor array. Nova relies on partially bound
                    // descriptor arrays
                    return true;
                }

                device = try_device;

                // Save information about the device
                D3D12_FEATURE_DATA_ARCHITECTURE arch;
                res = device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &arch, sizeof(D3D12_FEATURE_DATA_ARCHITECTURE));
                if(SUCCEEDED(res)) {
                    is_uma = arch.CacheCoherentUMA;
                }

                D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5;
                res = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5));
                if(SUCCEEDED(res)) {
                    render_pass_tier = options5.RenderPassesTier;
                    has_raytracing = options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
                }

                return false;
            }

            return true;
        });

        if(!device) {
            logger->error("Could not find a suitable D3D12 adapter");
        }
    }
} // namespace nova::renderer::rhi
