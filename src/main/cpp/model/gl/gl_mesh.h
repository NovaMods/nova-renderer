/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_GL_VERTEX_BUFFER_H
#define RENDERER_GL_VERTEX_BUFFER_H


#include "interfaces/ivertex_buffer.h"
#include <glad/glad.h>
#include "model/physics/AABB.h"

namespace nova {
    namespace model {
        /*!
         * \brief Represents a buffer which holds vertex information
         *
         * Buffers of this type can hold positions, positions and texture coordinates, or positions, texture coordinates,
         * lightmap coordinates, normals, and tangents.
         */
        class gl_mesh : public ivertex_buffer {
        public:
            gl_mesh();

            ~gl_mesh();

            void create();

            void destroy();

            void set_data(std::vector<float> data, format data_format, usage data_usage);

            void set_index_array(std::vector<unsigned short> data, usage data_usage);

            void set_active();

            void draw();

        private:
            GLuint vertex_buffer;
            GLuint indices;

            GLenum translate_usage(const usage data_usage) const;

            /*!
             * \brief Enables all the proper OpenGL vertex attributes for the given format
             *
             * Enables the proper vertex attribute array bind points and the vertex attribute pointers
             */
            void enable_vertex_attributes(format data_format);

            unsigned int vertex_array;
            unsigned int num_indices;

            AABB aabb;

            /*!
             * \brief Examines the vertices in the given vector, finding the minimum and maximum bounds in each axis
             *
             * \param vertices The vertices the get the bounds of
             * \param data_format The format of the vertices. Necessary for knowing the desired stride
             */
            void compute_aabb(std::vector<float> &vertices, format data_format);
        };
    }
}

#endif //RENDERER_GL_VERTEX_BUFFER_H
