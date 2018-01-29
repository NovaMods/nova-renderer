/*!
 * \author David
 * \date 22-May-16.
 */

#ifndef RENDERER_GL_UNIFORM_BUFFER_H
#define RENDERER_GL_UNIFORM_BUFFER_H

#include <string>
#include <GLFW/glfw3.h>

namespace nova {
    class vk_shader_program;

    /*!
     * \brief A nice interface for uniform buffer objects
     */
    template <typename T>
    class gl_uniform_buffer {
    public:
        gl_uniform_buffer(std::string name) : name(name) {
            //glCreateBuffers(1, &gl_name);
            LOG(TRACE) << "creating ubo " << name << " with size: " << sizeof(T);
            //glNamedBufferStorage(gl_name, sizeof(T), nullptr, GL_DYNAMIC_STORAGE_BIT);
        }

        gl_uniform_buffer(gl_uniform_buffer &&old) noexcept {
            gl_name = old.gl_name;
            name = old.name;

            old.gl_name = 0;
            old.name = "";
        }

        void link_to_shader(const vk_shader_program &shader) {
            //auto ubo_index = glGetUniformBlockIndex(shader.gl_name, name.c_str());
            //glBindBuffer(GL_UNIFORM_BUFFER, gl_name);
            //glBindBufferBase(GL_UNIFORM_BUFFER, ubo_index, gl_name);
        }

        void send_data(T &data) {
            LOG(TRACE) << "sending date with size: " << sizeof(T) << " to ubo " << name;
            //glNamedBufferSubData(gl_name, 0, sizeof(T), &data);
        }

        void bind() {
            //glBindBuffer(GL_UNIFORM_BUFFER, gl_name);
        }

        /*!
         * \brief Deallocates this uniform buffer
         */
        ~gl_uniform_buffer() {
            if(glfwGetCurrentContext() != NULL) {
                //glDeleteBuffers(1, &gl_name);
            }
        }

    private:
        GLuint gl_name;
        std::string name;
    };
}

#endif //RENDERER_GL_UNIFORM_BUFFER_H
