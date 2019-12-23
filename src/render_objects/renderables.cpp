#include "nova_renderer/renderables.hpp"

#include <glm/ext/matrix_transform.inl>

namespace nova::renderer {
    StaticMeshRenderCommand make_render_command(const StaticMeshRenderableData& data, const RenderableId id) {
        StaticMeshRenderCommand command = {};
        command.id = id;
        command.is_visible = true;
        // TODO: Make sure this is accurate
        command.model_matrix = glm::translate(command.model_matrix, data.initial_position);
        command.model_matrix = glm::rotate(command.model_matrix, data.initial_rotation.x, {1, 0, 0});
        command.model_matrix = glm::rotate(command.model_matrix, data.initial_rotation.y, {0, 1, 0});
        command.model_matrix = glm::rotate(command.model_matrix, data.initial_rotation.z, {0, 0, 1});
        command.model_matrix = glm::scale(command.model_matrix, data.initial_scale); // Uniform scaling only

        return command;
    }
} // namespace nova::renderer
