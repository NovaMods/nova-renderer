#include "nova_renderer/rhi/rhi_types.hpp"

namespace nova::renderer::rhi {
    bool ResourceBindingDescription::operator==(const ResourceBindingDescription& other) {
        return set == other.set && binding == other.binding && count == other.count && type == other.type;
    }

    bool ResourceBindingDescription::operator!=(const ResourceBindingDescription& other) { return !(*this == other); }

    ShaderStage operator|=(const ShaderStage lhs, const ShaderStage rhs) {
        return static_cast<ShaderStage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }
} // namespace nova::renderer::rhi
