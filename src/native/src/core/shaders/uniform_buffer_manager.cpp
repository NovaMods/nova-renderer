/*!
 * \author David
 * \date 19-May-16.
 */

#include "uniform_buffer_manager.h"
#include "uniform_buffers.h"

uniform_buffer_manager::uniform_buffer_manager() {
    GLuint buffer_name;
    glGenBuffers(1, &buffer_name);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer_name);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(cameraData), NULL, GL_DYNAMIC_DRAW);

    uniform_buffers["cameraData"] = buffer_name;
}

