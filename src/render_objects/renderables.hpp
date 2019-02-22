//
// Created by jannis on 11.09.18.
//

#ifndef NOVA_RENDERER_RENDER_OBJECT_HPP
#define NOVA_RENDERER_RENDER_OBJECT_HPP

#include <vulkan/vulkan.h>

namespace nova::renderer {
    struct renderable_base {
        VkDescriptorSet model_matrix_ubo_descriptor;
    };

    struct vk_static_mesh_renderable : renderable_base {
        const VkDrawIndexedIndirectCommand* draw_cmd;
    };
} // namespace nova::renderer

#endif // NOVA_RENDERER_RENDER_OBJECT_HPP
