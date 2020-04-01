#pragma once

#include <d3d12.h>
#include <rx/core/string.h>

namespace nova::renderer::rhi {
    void set_object_name(ID3D12Object* object, const rx::string& name);
}
