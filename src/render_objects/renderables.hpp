//
// Created by jannis on 11.09.18.
//

#ifndef NOVA_RENDERER_RENDER_OBJECT_HPP
#define NOVA_RENDERER_RENDER_OBJECT_HPP

#include <vulkan/vulkan.h>
#include "../loading/shaderpack/shaderpack_data.hpp"
#include "../render_engine/render_engine.hpp"
#include "../render_engine/vulkan/fixed_size_buffer_allocator.hpp"

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

        fixed_size_buffer_allocator<sizeof(glm::mat4)>::block* model_matrix_slot = nullptr;
    };

    struct vk_static_mesh_renderable : renderable_base {};
} // namespace nova::renderer

#endif // NOVA_RENDERER_RENDER_OBJECT_HPP
