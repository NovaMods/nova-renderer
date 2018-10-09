//
// Created by jannis on 18.09.18.
//

#ifndef NOVA_RENDERER_VULKAN_TYPE_CONVERTERS_HPP
#define NOVA_RENDERER_VULKAN_TYPE_CONVERTERS_HPP

#include <vulkan/vulkan.h>
#include "../../loading/shaderpack/shaderpack_data.hpp"

#endif //NOVA_RENDERER_VULKAN_TYPE_CONVERTERS_HPP

namespace nova::vulkan {
    class type_converters {
    public:
        static VkPrimitiveTopology primitive_topology(primitive_topology_enum topology) {
            switch (topology) {
                case primitive_topology_enum::Lines:
                    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                case primitive_topology_enum::Triangles:
                    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            }
        }

        static VkBlendFactor blend_factor(blend_factor_enum factor) {
            switch (factor) {
                case blend_factor_enum::DstAlpha:
                    return VK_BLEND_FACTOR_DST_ALPHA;
                case blend_factor_enum::DstColor:
                    return VK_BLEND_FACTOR_DST_COLOR;
                case blend_factor_enum::One:
                    return VK_BLEND_FACTOR_ONE;
                case blend_factor_enum::OneMinusDstAlpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
                case blend_factor_enum::OneMinusDstColor:
                    return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
                case blend_factor_enum::OneMinusSrcAlpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                case blend_factor_enum::OneMinusSrcColor:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
                case blend_factor_enum::SrcAlpha:
                    return VK_BLEND_FACTOR_SRC_ALPHA;
                case blend_factor_enum::SrcColor:
                    return VK_BLEND_FACTOR_SRC_COLOR;
                case blend_factor_enum::Zero:
                    return VK_BLEND_FACTOR_ZERO;
            }
        }
    };
}