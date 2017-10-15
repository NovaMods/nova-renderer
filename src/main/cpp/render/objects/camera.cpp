/*! 
 * \author gold1 
 * \date 01-Jun-17.
 */

#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

namespace nova {
    glm::mat4& camera::get_projection_matrix() {
        if(projection_matrix_is_dirty) {
            projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
            projection_matrix_is_dirty = false;
        }

        return projection_matrix;
    }

    glm::mat4 camera::get_view_matrix() {
        glm::mat4 view_matrix;
        view_matrix = glm::rotate(view_matrix, glm::radians(180.0f), {0, 1, 0});
        view_matrix = glm::rotate(view_matrix, glm::radians(-rotation.y), { 1, 0, 0 });
        view_matrix = glm::rotate(view_matrix, glm::radians(rotation.x), { 0, 1, 0 });
        view_matrix = glm::translate(view_matrix, -position);

        return view_matrix;
    }

    glm::vec3 camera::get_view_direction() {
        glm::mat4 view_matrix;
        view_matrix = glm::rotate(view_matrix, glm::radians(180.0f), {0, 1, 0});
        view_matrix = glm::rotate(view_matrix, glm::radians(-rotation.y), { 1, 0, 0 });
        view_matrix = glm::rotate(view_matrix, glm::radians(rotation.x), { 0, 1, 0 });

        return glm::mat3(view_matrix) * glm::vec3{0, 0, 1};
    }

    void camera::recalculate_frustum() {
        glm::mat4 proj = get_projection_matrix();
        glm::mat4 modl = get_view_matrix();
        glm::mat4 clip;

        float t;

        clip = proj * modl;

        /* Extract the numbers for the RIGHT plane */
        frustum[0][0] = clip[0][3] - clip[0][0];
        frustum[0][1] = clip[1][3] - clip[1][0];
        frustum[0][2] = clip[2][3] - clip[2][0];
        frustum[0][3] = clip[3][3] - clip[3][0];

        /* Normalize the result */
        t = std::sqrt(frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2]);
        frustum[0][0] /= t;
        frustum[0][1] /= t;
        frustum[0][2] /= t;
        frustum[0][3] /= t;

        /* Extract the numbers for the LEFT plane */
        frustum[1][0] = clip[0][3] + clip[0][0];
        frustum[1][1] = clip[1][3] + clip[1][0];
        frustum[1][2] = clip[2][3] + clip[2][0];
        frustum[1][3] = clip[3][3] + clip[3][0];

        /* Normalize the result */
        t = std::sqrt(frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2]);
        frustum[1][0] /= t;
        frustum[1][1] /= t;
        frustum[1][2] /= t;
        frustum[1][3] /= t;

        /* Extract the BOTTOM plane */
        frustum[2][0] = clip[0][3] + clip[0][1];
        frustum[2][1] = clip[1][3] + clip[1][1];
        frustum[2][2] = clip[2][3] + clip[2][1];
        frustum[2][3] = clip[3][3] + clip[3][1];

        /* Normalize the result */
        t = std::sqrt(frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2]);
        frustum[2][0] /= t;
        frustum[2][1] /= t;
        frustum[2][2] /= t;
        frustum[2][3] /= t;

        /* Extract the TOP plane */
        frustum[3][0] = clip[0][3] - clip[0][1];
        frustum[3][1] = clip[1][3] - clip[1][1];
        frustum[3][2] = clip[2][3] - clip[2][1];
        frustum[3][3] = clip[3][3] - clip[3][1];

        /* Normalize the result */
        t = std::sqrt(frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2]);
        frustum[3][0] /= t;
        frustum[3][1] /= t;
        frustum[3][2] /= t;
        frustum[3][3] /= t;

        /* Extract the FAR plane */
        frustum[4][0] = clip[0][3] - clip[0][2];
        frustum[4][1] = clip[1][3] - clip[1][2];
        frustum[4][2] = clip[2][3] - clip[2][2];
        frustum[4][3] = clip[3][3] - clip[3][2];

        /* Normalize the result */
        t = std::sqrt(frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2]);
        frustum[4][0] /= t;
        frustum[4][1] /= t;
        frustum[4][2] /= t;
        frustum[4][3] /= t;

        /* Extract the NEAR plane */
        frustum[5][0] = clip[0][3] + clip[0][2];
        frustum[5][1] = clip[1][3] + clip[1][2];
        frustum[5][2] = clip[2][3] + clip[2][2];
        frustum[5][3] = clip[3][3] + clip[3][2];

        /* Normalize the result */
        t = std::sqrt(frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2]);
        frustum[5][0] /= t;
        frustum[5][1] /= t;
        frustum[5][2] /= t;
        frustum[5][3] /= t;
    }

    bool camera::has_object_in_frustum(aabb &bounding_box) {
        float x = bounding_box.center.x;
        float y = bounding_box.center.y;
        float z = bounding_box.center.z;

        float xSize = bounding_box.extents.x;
        float ySize = bounding_box.extents.y;
        float zSize = bounding_box.extents.z;

        int p;

        for( p = 0; p < 6; p++ )
        {
            if( frustum[p][0] * (x - xSize) + frustum[p][1] * (y - ySize) + frustum[p][2] * (z - zSize) + frustum[p][3] > 0 )
                continue;
            if( frustum[p][0] * (x + xSize) + frustum[p][1] * (y - ySize) + frustum[p][2] * (z - zSize) + frustum[p][3] > 0 )
                continue;
            if( frustum[p][0] * (x - xSize) + frustum[p][1] * (y + ySize) + frustum[p][2] * (z - zSize) + frustum[p][3] > 0 )
                continue;
            if( frustum[p][0] * (x + xSize) + frustum[p][1] * (y + ySize) + frustum[p][2] * (z - zSize) + frustum[p][3] > 0 )
                continue;
            if( frustum[p][0] * (x - xSize) + frustum[p][1] * (y - ySize) + frustum[p][2] * (z + zSize) + frustum[p][3] > 0 )
                continue;
            if( frustum[p][0] * (x + xSize) + frustum[p][1] * (y - ySize) + frustum[p][2] * (z + zSize) + frustum[p][3] > 0 )
                continue;
            if( frustum[p][0] * (x - xSize) + frustum[p][1] * (y + ySize) + frustum[p][2] * (z + zSize) + frustum[p][3] > 0 )
                continue;
            if( frustum[p][0] * (x + xSize) + frustum[p][1] * (y + ySize) + frustum[p][2] * (z + zSize) + frustum[p][3] > 0 )
                continue;
            return false;
        }
        return true;
    }
}
