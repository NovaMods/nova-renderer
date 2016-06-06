/*!
 * \brief Defines a number of UBOs that are sent to shaders
 *
 * \author David
 * \date 19-May-16.
 */

#ifndef RENDERER_UNIFORM_BUFFERS_H
#define RENDERER_UNIFORM_BUFFERS_H

#include <gl/glm/glm.hpp>

/*!
 * \brief Holds data that changes whenever the camera changes
 *
 * Used by every non-gui shader
 */
struct camera_data {
    glm::mat4 gbufferModelView;   //!< This is actually the view matrix, but it was super poorly named :(
    glm::mat4 gbufferProjection;
    glm::mat4 gbufferModelviewInverse;
    glm::mat4 gbufferProjectionInverse;
    glm::vec3 cameraPosition;
};

#endif //RENDERER_UNIFORM_BUFFERS_H
