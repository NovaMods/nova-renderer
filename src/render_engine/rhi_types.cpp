#include "nova_renderer/rhi/rhi_types.hpp"

namespace nova::renderer::rhi {
    bool ResourceBindingDescription::operator==(const ResourceBindingDescription& other) {
        return set == other.set && binding == other.binding && count == other.count && type == other.type;
    }

    bool ResourceBindingDescription::operator!=(const ResourceBindingDescription& other) { return !(*this == other); }

    ShaderStageFlags operator|=(const ShaderStageFlags lhs, const ShaderStageFlags rhs) {
        return static_cast<ShaderStageFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }
} // namespace nova::renderer::rhi
