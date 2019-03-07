//
// Created by jannis on 11.09.18.
//

#ifndef NOVA_RENDERER_RENDER_OBJECT_HPP
#define NOVA_RENDERER_RENDER_OBJECT_HPP

#include <vulkan/vulkan.h>
#include "../loading/shaderpack/shaderpack_data.hpp"
#include "../render_engine/render_engine.hpp"

namespace nova::renderer {
    static std::atomic<renderable_id_t> next_renderable_id;

    struct renderable_metadata {
        renderable_id_t id = 0;

        std::vector<std::string> passes;
        VkBuffer buffer = nullptr;
    };

    struct renderable_base {
        renderable_id_t id = 0;

        bool is_visible = true;

        uint32_t matrix_index = 0;
    };

    struct vk_static_mesh_renderable : renderable_base {
    };
} // namespace nova::renderer

#endif // NOVA_RENDERER_RENDER_OBJECT_HPP
