/*!
 * \author ddubois 
 * \date 30-Aug-18.
 */

#include "dx_12_render_engine.hpp"

namespace nova {
    dx12_render_engine::dx12_render_engine(const settings &settings) : render_engine(settings) {
        create_device();
    }

    const std::string dx12_render_engine::get_engine_name() const {
        return "DirectX 12";
    }

    void dx12_render_engine::create_device() {
        HRESULT hr;

        IDXGIFactory2* dxgi_factory;
        hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory));
        if(FAILED(hr)) {
            throw std::runtime_error("Could not create DXGI Factory");
        }

        IDXGIAdapter1* adapter;

        int adapter_index = 0;
        bool adapter_found = false;

        while(dxgi_factory->EnumAdapters1(adapter_index, &adapter) != DXGI_ERROR_NOT_FOUND) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                // Ignore software devices
                adapter_index++;
                continue;
            }

            hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
            if(SUCCEEDED(hr)) {
                adapter_found = true;
                break;
            }

            adapter_index++;
        }

        if(!adapter_found) {
            throw std::runtime_error("Could not find a GPU that supports DX12");
        }
    }
}
