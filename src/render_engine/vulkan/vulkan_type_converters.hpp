//
// Created by jannis on 18.09.18.
//

#ifndef NOVA_RENDERER_VULKAN_TYPE_CONVERTERS_HPP
#define NOVA_RENDERER_VULKAN_TYPE_CONVERTERS_HPP

#include <vulkan/vulkan.h>
#include "../../../include/nova_renderer/shaderpack_data.hpp"

#ifdef Always // RIP X11 and its macros
#undef Always
#endif

#endif // NOVA_RENDERER_VULKAN_TYPE_CONVERTERS_HPP

namespace nova::renderer::vulkan {
    class type_converters {
    public:
        static VkPrimitiveTopology to_primitive_topology(primitive_topology_enum topology) {
            switch(topology) {
                case primitive_topology_enum::Lines:
                    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                case primitive_topology_enum::Triangles:
                    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            }
        }

        static VkBlendFactor to_blend_factor(blend_factor_enum factor) {
            switch(factor) {
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
                default:
                    return VK_BLEND_FACTOR_ZERO;
            }
        }

        static VkCompareOp to_compare_op(const compare_op_enum compare_op) {
            switch(compare_op) {
                case compare_op_enum::Never:
                    return VK_COMPARE_OP_NEVER;

                case compare_op_enum::Less:
                    return VK_COMPARE_OP_LESS;

                case compare_op_enum::LessEqual:
                    return VK_COMPARE_OP_LESS_OR_EQUAL;

                case compare_op_enum::Greater:
                    return VK_COMPARE_OP_GREATER;

                case compare_op_enum::GreaterEqual:
                    return VK_COMPARE_OP_GREATER_OR_EQUAL;

                case compare_op_enum::Equal:
                    return VK_COMPARE_OP_EQUAL;

                case compare_op_enum::NotEqual:
                    return VK_COMPARE_OP_NOT_EQUAL;

                case compare_op_enum::Always:
                    return VK_COMPARE_OP_ALWAYS;

                default:
                    return VK_COMPARE_OP_NEVER;
            }
        }

        static VkStencilOp to_stencil_op(stencil_op_enum stencil_op) {
            switch(stencil_op) {
                case stencil_op_enum::Keep:
                    return VK_STENCIL_OP_KEEP;

                case stencil_op_enum::Zero:
                    return VK_STENCIL_OP_ZERO;

                case stencil_op_enum::Replace:
                    return VK_STENCIL_OP_REPLACE;

                case stencil_op_enum::Incr:
                    return VK_STENCIL_OP_INCREMENT_AND_CLAMP;

                case stencil_op_enum::IncrWrap:
                    return VK_STENCIL_OP_INCREMENT_AND_WRAP;

                case stencil_op_enum::Decr:
                    return VK_STENCIL_OP_DECREMENT_AND_CLAMP;

                case stencil_op_enum::DecrWrap:
                    return VK_STENCIL_OP_DECREMENT_AND_WRAP;

                case stencil_op_enum::Invert:
                    return VK_STENCIL_OP_INVERT;

                default:
                    return VK_STENCIL_OP_KEEP;
            }
        }
    };
} // namespace nova::renderer::vulkan