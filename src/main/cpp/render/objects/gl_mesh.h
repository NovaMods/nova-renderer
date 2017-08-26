/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_GL_VERTEX_BUFFER_H
#define RENDERER_GL_VERTEX_BUFFER_H

#include <glad/glad.h>
#include <vector>
#include "../../geometry_cache/mesh_definition.h"
#include "../../data_loading/physics/AABB.h"

namespace nova {
    /*!
     * \brief Specifies how the data in thie buffer will be used
     */
    enum class usage {
        /*!
         * \brief The buffer will be updated once and drawn many times
         */
                static_draw,

        /*!
         * \brief The buffer will be updated many times and drawn many times
         */
                dynamic_draw,
    };

    /*!
     * \brief Represents a buffer which holds vertex information
     *
     * Buffers of this type can hold positions, positions and texture coordinates, or positions, texture coordinates,
     * lightmap coordinates, normals, and tangents.
     */
    class gl_mesh {
    public:
        gl_mesh();

        explicit gl_mesh(const mesh_definition &definition);

        ~gl_mesh();

        void create();

        void destroy();

        /*!
         * \brief Sets the given data as this vertex buffer's data, and uploads that data to the GPU
         *
         * \param data The interleaved vertex data
         * \param data_format The format of the data (\see format)
         */
        void set_data(std::vector<int> data, format data_format, usage data_usage);

        void set_index_array(std::vector<int> data, usage data_usage);

        void set_active() const;

        void draw() const;

        /*!
         * \brief Returns the format of this vertex buffer
         *
         * \return The format of this vertex buffer
         */
        format get_format();

        bool has_data() const;

    private:
        format data_format;

        GLuint vertex_buffer;
        GLuint indices;

        GLenum translate_usage(usage data_usage) const;

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
        void compute_aabb(std::vector<int> &vertices, format data_format);
    };
}

#endif //RENDERER_GL_VERTEX_BUFFER_H
