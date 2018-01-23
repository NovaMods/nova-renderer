//
// Created by ddubois on 8/8/17.
//

#include "render_object.h"
#include "meshes/vk_mesh.h"

namespace nova {
    render_object::render_object(render_object &&other) noexcept {
        parent_id = other.parent_id;
        type = other.type;
        geometry = std::move(other.geometry);
        color_texture = std::move(other.color_texture);
        normalmap = std::move(other.normalmap);
        data_texture = std::move(other.data_texture);
        position = other.position;

        other.parent_id = 0;
        other.geometry.reset();
        other.normalmap = std::experimental::optional<std::string>();
        other.data_texture = std::experimental::optional<std::string>();
        other.position = {0, 0, 0};
    }

    render_object &render_object::operator=(render_object && other) noexcept {
        parent_id = other.parent_id;
        type = other.type;
        geometry = std::move(other.geometry);
        color_texture = std::move(other.color_texture);
        normalmap = std::move(other.normalmap);
        data_texture = std::move(other.data_texture);
        position = other.position;

        other.parent_id = 0;
        other.geometry.reset();
        other.normalmap = std::experimental::optional<std::string>();
        other.data_texture = std::experimental::optional<std::string>();
        other.position = {0, 0, 0};

        return *this;
    }
}