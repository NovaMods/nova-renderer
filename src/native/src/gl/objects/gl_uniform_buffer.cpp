/*!
 * \author David
 * \date 22-May-16.
 */

#include "gl_uniform_buffer.h"

gl_uniform_buffer::gl_uniform_buffer(GLuint size) {
    glGenBuffers(1, &gl_name);
    bind();
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
}

gl_uniform_buffer::~gl_uniform_buffer() {
    glDeleteBuffers(1, &gl_name);
}

void gl_uniform_buffer::bind() {
    glBindBuffer(GL_UNIFORM_BUFFER, gl_name);
}

void gl_uniform_buffer::set_bind_point(GLuint bind_point) {
    glBindBufferBase(GL_UNIFORM_BUFFER, bind_point, gl_name);
}







