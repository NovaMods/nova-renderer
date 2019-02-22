//
// Created by jannis on 11.09.18.
//

#ifndef NOVA_RENDERER_RENDER_OBJECT_HPP
#define NOVA_RENDERER_RENDER_OBJECT_HPP

#include <vulkan/vulkan.h>
#include "../loading/shaderpack/shaderpack_data.hpp"
#include "../render_engine/render_engine.hpp"

namespace nova::renderer {
    static std::atomic<renderable_id_t> RENDERABLE_ID;

    struct renderable_metadata {
        renderable_id_t id;

        std::vector<const material_pass*> passes;
    };

    struct renderable_base {
        renderable_id_t id;

        bool is_visible = true;

        VkDescriptorSet model_matrix_ubo_descriptor;
    };

    struct vk_static_mesh_renderable : renderable_base {
        const VkDrawIndexedIndirectCommand* draw_cmd;
    };
} // namespace nova::renderer

#endif // NOVA_RENDERER_RENDER_OBJECT_HPP
