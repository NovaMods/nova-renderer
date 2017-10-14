/*! 
 * \author gold1 
 * \date 01-Jun-17.
 */

#ifndef RENDERER_CAMERA_H
#define RENDERER_CAMERA_H

#include <glm/glm.hpp>
#include "../../data_loading/physics/AABB.h"

namespace nova {
    /*!
     * \brief A camera that can render things.
     *
     * There's one camera for the shadow map and one camera for the player. Other cameras are possible but I don't wanna
     */
    struct camera {
        float fov = 75;
        float aspect_ratio = 16.f / 9.f;
        float near_plane = 0.01f;
        float far_plane = 1000.f;
        glm::vec2 rotation;
        glm::vec3 position;

        glm::mat4& get_projection_matrix();
        glm::mat4 get_view_matrix();

        glm::vec3 get_view_direction();

        void recalculate_frustum();

        bool has_object_in_frustum(AABB& bounding_box);

    private:
        bool projection_matrix_is_dirty;

        glm::mat4 projection_matrix;

        float frustum[6][4];
    };
}


#endif //RENDERER_CAMERA_H
