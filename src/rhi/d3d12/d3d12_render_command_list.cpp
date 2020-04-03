#include "d3d12_render_command_list.hpp"

#include "d3d12_resource_binder.hpp"
#include "d3d12_utils.hpp"

namespace nova::renderer::rhi {
    D3D12RenderCommandList::D3D12RenderCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmds)
        : command_list{rx::utility::move(cmds)} {
        command_list->QueryInterface(command_list_4.GetAddressOf());
    }

    void D3D12RenderCommandList::set_debug_name(const rx::string& name) { set_object_name(command_list.Get(), name); }

    void D3D12RenderCommandList::set_checkpoint(const rx::string& checkpoint_name) {
        // D3D12 is lame with this
        // You can use `ID3D12GraphicsCommandList2::WriteBufferImmediate` to write a specific value to a specific GPU virtual address. I
        // definitely could make a buffer that holds the values for each command list, and use WriteBufferImmediate to write values to that
        // buffer, and implement some machinery in D3D12RenderDevice to read back that buffer value... but I want to stay in
        // D3D12RenderCommandList tonight, so I wrote this comment instead
    }

    void D3D12RenderCommandList::bind_resources(RhiResourceBinder& binder) {
        auto& d3d12_binder = static_cast<D3D12ResourceBinder&>(binder);

        command_list->SetGraphicsRootSignature(d3d12_binder.get_root_signature());
    }

} // namespace nova::renderer::rhi
