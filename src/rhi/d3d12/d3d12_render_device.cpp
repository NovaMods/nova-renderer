#include "d3d12_render_device.hpp"


namespace nova::renderer ::rhi {
    D3D12RenderDevice::D3D12RenderDevice(NovaSettingsAccessManager& settings, NovaWindow& window, rx::memory::allocator& allocator)
        : RenderDevice(settings, window, allocator) {
        initialize_dxgi();

        select_adapter();
    }

    void D3D12RenderDevice::initialize_dxgi() {
        CreateDXGIFactory(IID_PPV_ARGS(&factory));
    }

    void D3D12RenderDevice::select_adapter() {
        // We want an adapter:
        // - Not integrated, if possible

        UINT adapter_idx = 0;
        IDXGIAdapter* adapter;
        while(factory->EnumAdapters(adapter_idx, &adapter) != DXGI_ERROR_NOT_FOUND) {

            adapter_idx++;
        }
    }
} // namespace nova::renderer::rhi
