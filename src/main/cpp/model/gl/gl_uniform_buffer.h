/*!
 * \author David
 * \date 22-May-16.
 */

#ifndef RENDERER_GL_UNIFORM_BUFFER_H
#define RENDERER_GL_UNIFORM_BUFFER_H

#include <string>
#include <glad/glad.h>

namespace nova {
    namespace model {
/*!
 * \brief Represents a uniform buffer, which can be used for whatever
 */
        class gl_uniform_buffer {
        public:
            /*!
             * \brief Initializes this uniform buffer, creating it on the GPU and whatnot
             *
             * \param size The number of bytes to allocate for this buffer
             */
            gl_uniform_buffer(GLuint size);

            gl_uniform_buffer() {};

            gl_uniform_buffer(gl_uniform_buffer &&old) noexcept;

            /*!
             * \brief Deallocates this uniform buffer
             */
            ~gl_uniform_buffer();

            /*!
             * \brief Binds this uniform buffer so we can do things to it
             *
             * \param bindpoint The buffer thing to bind this buffer to
             */
            void bind();

            /*!
             * \brief Binds this buffer to the given bind point
             *
             * \param bind_point The bind point to bind this buffer to
             */
            void set_bind_point(GLuint bind_point);

            void set_name(std::string name) noexcept;

            /*!
             * \brief Returns the binding point of this uniform block
             */
            GLuint get_bind_point() const noexcept;

            /*!
             * \brier Returns the OpenGL name for this uniform block
             */
            GLuint get_gl_name() const noexcept;

            const std::string &get_name() const noexcept;

            /*!
             * \brief Uploads the given data to this UBO
             *
             * Note that absolutely no checking is done to make sure you're uploading the right data. You better know what
             * you're doing.
             *
             * Note also that this function binds the UBO, so you don't need to bind it yourself.
             */
            template <typename T>
            void send_data(T data) {
                bind();
                GLvoid *p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);

                memcpy(p, &data, sizeof(T));
                //glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), &data);
            };

            void operator=(gl_uniform_buffer &&old) noexcept;

        private:
            GLuint gl_name;
            GLuint bind_point;
            std::string name;
        };
    }
}

#endif //RENDERER_GL_UNIFORM_BUFFER_H
