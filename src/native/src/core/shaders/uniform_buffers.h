/*!
 * \brief Defines a number of UBOs that are sent to shaders
 *
 * I've put them all in one header file so that there's a single place to see all the available uniforms. When this
 * project is a bit closer to release, I'll document this file so that the Doxygen generated documentation will serve
 * as a reference of what uniforms are available through Nova.
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
    glm::mat4 gbufferModelviewInverse;  //!< same as gbufferModelView
    glm::mat4 gbufferProjectionInverse;
    glm::vec3 cameraPosition;
    float viewWidth;
    float viewHeight;
};

#endif //RENDERER_UNIFORM_BUFFERS_H
