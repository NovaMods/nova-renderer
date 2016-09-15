//
// Created by David on 15-Apr-16.
//

#ifndef RENDERER_GLFW_GL_WINDOW_H
#define RENDERER_GLFW_GL_WINDOW_H


#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include "interfaces/iwindow.h"
#include "model/settings.h"

/*!
 * \brief Represents a GLFW window with an OpenGL context
 *
 * In OpenGL, the window is responsible for maintaining the OpenGL context, or the context has to be bound to a window,
 * or something. I'm not entirely sure, but I do know that the window and OpenGL context are pretty closely coupled.
 *
 * Point is, this class is pretty important and you shouldn't leave home without it
 */
class glfw_gl_window : public iwindow {
public:
    /*!
     * \brief Creates a window and a corresponding OpenGL context
     */
    glfw_gl_window();
    ~glfw_gl_window();

    /**
     * iwindow methods
     */

    virtual int init();
    virtual void destroy();
    virtual void end_frame();
    virtual void set_fullscreen(bool fullscreen);
    virtual glm::vec2 get_size();
    virtual bool should_close();

    /**
     * iconfig_change_listener methods
     */

    void on_config_change(nlohmann::json& new_config);

    void on_config_loaded(nlohmann::json& config);
private:
    GLFWwindow *window;
    glm::ivec2 window_dimensions;

    void set_framebuffer_size(glm::ivec2 new_framebuffer_size);
};


#endif //RENDERER_GLFW_GL_WINDOW_H
