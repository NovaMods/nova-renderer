#include "d3d12_command_list.hpp"

#include "d3d12_resource_binder.hpp"

namespace nova::renderer::rhi {
    void D3D12CommandList::bind_resources(RhiResourceBinder& binder) {
        auto& d3d12_binder = static_cast<D3D12ResourceBinder&>(binder);

        command_list->SetGraphicsRootSignature(d3d12_binder.get_root_signature());
    }

} // namespace nova::renderer::rhi
