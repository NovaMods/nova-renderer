//
// Created by jannis on 11.09.18.
//

#ifndef NOVA_RENDERER_RENDER_OBJECT_HPP
#define NOVA_RENDERER_RENDER_OBJECT_HPP

#include <glm/glm.hpp>
#include <memory>
#include <list>

namespace nova {
    struct vk_mesh;

    struct render_object {
        vk_mesh* mesh;

        glm::vec3 pos;
    };
}  // namespace nova

#endif  // NOVA_RENDERER_RENDER_OBJECT_HPP
