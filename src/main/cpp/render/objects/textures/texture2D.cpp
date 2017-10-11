//
// Created by David on 26-Apr-16.
//

#include "texture2D.h"
#include <stdexcept>
#include <easylogging++.h>
#include "../../../utils/utils.h"

namespace nova {
    texture2D::texture2D() : height(0), width(0) {
        glGenTextures(1, &gl_name);
    }

    void texture2D::set_data(std::vector<float> &pixel_data, std::vector<int> &dimensions, GLenum format, GLenum internal_format) {
        if(dimensions.size() != 2) {
            // Someone wants to make a 2D texture without diving us 2 dimensions!
            throw std::invalid_argument("Can't create a texture2D without 2 dimensions!");
        }

        if(pixel_data.size() <= 512) {
            LOG(DEBUG) << "Pixel data: " << pixel_data;
        }

        GLint previous_texture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_texture);
        glBindTexture(GL_TEXTURE_2D, gl_name);
        glTexImage2D(GL_TEXTURE_2D, 0, format, dimensions[0], dimensions[1], 0, format, internal_format, pixel_data.data());

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glBindTexture(GL_TEXTURE_2D, (GLuint) previous_texture);
    }

    void texture2D::bind(unsigned int binding) {
        glBindTextureUnit(binding, gl_name);
        current_location = binding;
    }

    void texture2D::unbind() {
        glActiveTexture((GLenum) current_location);
        glBindTexture(GL_TEXTURE_2D, 0);
        current_location = -1;
    }

    int texture2D::get_width() {
        return width;
    }

    int texture2D::get_height() {
        return height;
    }

    GLint texture2D::get_format() {
        return format;
    }

    void texture2D::set_filtering_parameters(texture_filtering_params &params) {
        // TODO
    }

    const unsigned int &texture2D::get_gl_name() {
        return gl_name;
    }

    void texture2D::set_name(const std::string name) {
        this->name = name;
    }

    const std::string &texture2D::get_name() const {
        return name;
    }
}
