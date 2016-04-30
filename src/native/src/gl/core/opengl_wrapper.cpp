//
// Created by David on 01-Apr-16.
//

#include "opengl_wrapper.h"
#include "../glad/glad.h"

#include <vector>
#include <easylogging++.h>

opengl_wrapper::opengl_wrapper() {
    gladLoadGL();
}

texture2D opengl_wrapper::make_texture_2D() {
    GLuint gl_texture_name;
    glGenTextures(1, &gl_texture_name);
    return texture2D(gl_texture_name);
}


