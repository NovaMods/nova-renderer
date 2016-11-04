/*!
 * \author David
 * \date 22-May-16.
 */

#include <easylogging++.h>

#include "gl_uniform_buffer.h"

namespace nova {
    namespace model {
        gl_uniform_buffer::gl_uniform_buffer(GLuint size) {
            glGenBuffers(1, &gl_name);
            bind();
            glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
        }

        gl_uniform_buffer::gl_uniform_buffer(gl_uniform_buffer &&old) noexcept {
            gl_name = old.gl_name;
            bind_point = old.bind_point;
            old.gl_name = 0;
            old.bind_point = 0;
        }

        gl_uniform_buffer::~gl_uniform_buffer() {
            LOG(TRACE) << "Deleting buffer " << gl_name;
            glDeleteBuffers(1, &gl_name);
        }

        void gl_uniform_buffer::bind() {
            LOG(TRACE) << "Binding buffer " << gl_name;
            glBindBuffer(GL_UNIFORM_BUFFER, gl_name);
        }

        void gl_uniform_buffer::set_bind_point(GLuint bind_point) {
            LOG(TRACE) << "Setting buffer " << gl_name << " to bind point " << bind_point;
            glBindBufferBase(GL_UNIFORM_BUFFER, bind_point, gl_name);
            this->bind_point = bind_point;
        }

        void gl_uniform_buffer::operator=(gl_uniform_buffer &&old) noexcept {
            gl_name = old.gl_name;
            bind_point = old.bind_point;
            old.gl_name = 0;
            old.bind_point = 0;
        }

        void gl_uniform_buffer::set_name(std::string name) noexcept {
            this->name = name;
        }

        const std::string &gl_uniform_buffer::get_name() const noexcept {
            return name;
        }

        GLuint gl_uniform_buffer::get_bind_point() const noexcept {
            return bind_point;
        }

        GLuint gl_uniform_buffer::get_gl_name() const noexcept {
            return gl_name;
        }
    }
}


