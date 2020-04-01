#include "d3d12_utils.hpp"

namespace nova::renderer::rhi {
    void set_object_name(ID3D12Object* object, const rx::string& name) {
        const auto wide_name = name.to_utf16();

        object->SetName(reinterpret_cast<LPCWSTR>(wide_name.data()));
    }
} // namespace nova::renderer::rhi
