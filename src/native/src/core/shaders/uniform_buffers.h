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
struct cameraData {
    glm::mat4 gbufferModelView;   //!< This is actually the view matrix, but it was super poorly named :(
    glm::mat4 gbufferProjection;
    glm::mat4 gbufferModelviewInverse;
    glm::mat4 gbufferProjectionInverse;
    glm::vec3 cameraPosition;
};

/*!
 * \brief Contains all the data needed to render the GUI
 *
 * I don't expect this to be very complicated, when all's said and done The GUI is pretty simple. I can update the
 * vertex buffer itself to have the texture coordinates of whatever texture a button needs. If people want, they can
 * use the other uniform buffers
 *
 * I don't expect the GUI to need updating very frequently, so I might be able to move this to a uniform if I need the
 * binding pint for anything else
 */
struct guiCameraData {
    glm::mat4 mvp;
};

#endif //RENDERER_UNIFORM_BUFFERS_H
