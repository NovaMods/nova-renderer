/*!
 * \author David
 * \date 13-May-16.
 */

#include <c++/4.8.3/stdexcept>
#include <easylogging++.h>
#include "gl_vertex_buffer.h"

gl_vertex_buffer::gl_vertex_buffer() {
    vertex_array = 0xFFFFFFFF;
    vertex_buffer = 0xFFFFFFFF;
    indices = 0xFFFFFFFF;
    create();
}

gl_vertex_buffer::~gl_vertex_buffer() {
    destroy();
}

void gl_vertex_buffer::create() {
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glGenBuffers(1, &vertex_buffer);
    glGenBuffers(1, &indices);
}

void gl_vertex_buffer::destroy() {
    if(vertex_buffer != 0xFFFFFFFF) {
        glDeleteBuffers(1, &vertex_buffer);
        vertex_buffer = 0xFFFFFFFF;
    }

    if(indices != 0xFFFFFFFF) {
        glDeleteBuffers(1, &indices);
        indices = 0xFFFFFFFF;
    }
}

void gl_vertex_buffer::set_data(std::vector<float> data, format data_format, usage data_usage) {
    this->data_format = data_format;

    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    GLenum buffer_usage = translate_usage(data_usage);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), buffer_usage);

    enable_vertex_attributes(data_format);
}

GLenum gl_vertex_buffer::translate_usage(const usage data_usage) const {
    switch(data_usage) {
        case usage::dynamic_draw:
            return GL_DYNAMIC_DRAW;
        case usage::static_draw:
            return GL_STATIC_DRAW;
        default:
            // In case something bad happens
            throw std::invalid_argument("data_usage value unsupported");
    }
}

void gl_vertex_buffer::set_active() {
    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
}

void gl_vertex_buffer::set_index_array(std::vector<unsigned short> data, usage data_usage) {
    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
    GLenum buffer_usage = translate_usage(data_usage);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned short), data.data(), buffer_usage);

    num_indices = (unsigned int) data.size();
}

void gl_vertex_buffer::draw() {
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, 0);
}

void gl_vertex_buffer::enable_vertex_attributes(format data_format) {
    switch(data_format) {
        case format::POS:
            // We only need to set up positional data
            // Positions are always at vertex attribute 0
            glEnableVertexAttribArray(0);   // Position

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            break;

        case format::POS_UV:
            glEnableVertexAttribArray(0);   // Position
            glEnableVertexAttribArray(1);   // Texture UV

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);

            break;

        case format::POS_UV_LIGHTMAPUV_NORMAL_TANGENT:
            glEnableVertexAttribArray(0);   // Position
            glEnableVertexAttribArray(1);   // Texture UV
            glEnableVertexAttribArray(2);   // Lightmap UV
            glEnableVertexAttribArray(3);   // Normal
            glEnableVertexAttribArray(4);   // Tangent

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(GL_FLOAT), (void*)0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 13 * sizeof(GL_FLOAT), (void*)0);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 13 * sizeof(GL_FLOAT), (void*)0);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 13 * sizeof(GL_FLOAT), (void*)0);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(GL_FLOAT), (void*)0);

            break;
    }
}
