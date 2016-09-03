/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_GL_VERTEX_BUFFER_H
#define RENDERER_GL_VERTEX_BUFFER_H


#include "interfaces/ivertex_buffer.h"
#include <glad/glad.h>

/*!
 * \brief Represents a buffer which holds vertex information
 *
 * Buffers of this type can hold positions, positions and texture coordinates, or positions, texture coordinates,
 * lightmap coordinates, normals, and tangents.
 */
class gl_vertex_buffer : public ivertex_buffer {
public:
    gl_vertex_buffer();
    ~gl_vertex_buffer();

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
};


#endif //RENDERER_GL_VERTEX_BUFFER_H
