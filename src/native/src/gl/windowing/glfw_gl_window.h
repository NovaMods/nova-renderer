//
// Created by David on 15-Apr-16.
//

#ifndef RENDERER_GLFW_GL_WINDOW_H
#define RENDERER_GLFW_GL_WINDOW_H


#include <GLFW/glfw3.h>
#include "../../interfaces/iwindow.h"

class glfw_gl_window : public iwindow {
public:
    glfw_gl_window();
    ~glfw_gl_window();

    virtual int init();
    virtual void destroy();
    virtual void end_frame();
    virtual void set_fullscreen(bool fullscreen);
    virtual glm::vec2 get_size();
    virtual bool should_close();
private:
    GLFWwindow *window;
    glm::ivec2 window_dimensions;
};


#endif //RENDERER_GLFW_GL_WINDOW_H
