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
    };
}