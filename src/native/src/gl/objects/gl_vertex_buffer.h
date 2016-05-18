/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_GL_VERTEX_BUFFER_H
#define RENDERER_GL_VERTEX_BUFFER_H


#include "../../interfaces/ivertex_buffer.h"
#include "../glad/glad.h"

class gl_vertex_buffer : public ivertex_buffer {
public:
    gl_vertex_buffer();
    ~gl_vertex_buffer();

    virtual void create();

    virtual void destroy();

    virtual void set_data(std::vector<float> data, format data_format, usage data_usage);

    virtual void set_index_array(std::vector<unsigned short> data, usage data_usage);

    virtual void set_active();

    virtual void draw();
private:
    GLuint gl_name;
    GLuint element_array_name;

    GLenum translate_usage(const usage data_usage) const;
};


#endif //RENDERER_GL_VERTEX_BUFFER_H
