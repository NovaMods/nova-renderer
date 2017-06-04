/*! 
 * \author gold1 
 * \date 01-Jun-17.
 */

#ifndef RENDERER_CAMERA_H
#define RENDERER_CAMERA_H

#include <glm/glm.hpp>

namespace nova {
    /*!
     * \brief A camera that can render things.
     *
     * There's one camera for the shadow map and one camera for the player. Other cameras are possible but i don't wanna
     */
    struct camera {
    public:
    private:
        float fov;
        float aspect_ratio;
        float near_plane;
        float far_plane;
        glm::vec2 rotation;

        glm::mat4 get_projection_matrix();
        glm::mat4 get_view_matrix();
    };
}


#endif //RENDERER_CAMERA_H
