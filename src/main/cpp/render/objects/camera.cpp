/*! 
 * \author gold1 
 * \date 01-Jun-17.
 */

#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace nova {
    glm::mat4 camera::get_projection_matrix() {
        return glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
    }

    glm::mat4 camera::get_view_matrix() {
        glm::mat4 view_matrix;
        view_matrix = glm::rotate(view_matrix, glm::radians(rotation.y), { 1, 0, 0 });
        view_matrix = glm::rotate(view_matrix, glm::radians(rotation.x), { 0, 1, 0 });
        view_matrix = glm::translate(view_matrix, -position);

        return view_matrix;
    }
}
