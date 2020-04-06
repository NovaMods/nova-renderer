#pragma once

namespace nova::renderer::rhi {
    enum class D3D12RootSignatureSlotType {
        CBV,
        SRV,
        UAV,
        DescriptorTable,
    };
}
