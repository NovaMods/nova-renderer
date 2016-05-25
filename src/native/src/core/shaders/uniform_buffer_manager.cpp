/*!
 * \author David
 * \date 19-May-16.
 */

#include "uniform_buffer_manager.h"
#include "uniform_buffers.h"

uniform_buffer_manager::uniform_buffer_manager() {
    uniform_buffers["cameraData"] = new gl_uniform_buffer(sizeof(cameraData));
    uniform_buffers["guiCameraData"] = new gl_uniform_buffer(sizeof(guiCameraData));
}

gl_uniform_buffer *uniform_buffer_manager::get_buffer(std::string buffer_name) {
    return uniform_buffers[buffer_name];
}



