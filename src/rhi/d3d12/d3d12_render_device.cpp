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
            const auto res = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&try_device));
            if(SUCCEEDED(res)) {
                // We created a device with the feature level we need, no need to enumerate other adapters
                device = try_device;

                return false;
            }

            return true;
        });

        if(!device) {
            logger->error("Could not find a suitable D3D12 adapter");
        }
    }
} // namespace nova::renderer::rhi
