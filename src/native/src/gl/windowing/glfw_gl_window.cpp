//
// Created by David on 15-Apr-16.
//

#include "glfw_gl_window.h"
#include "../../io/key_forwarder.h"
#include <easylogging++.h>


void error_callback(int error, const char * description) {
    LOG(ERROR) << "Error " << error << ": " << description;
}

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
    // TODO: Send keys to an input manager of some sort, where they can be sent on to some java code
    key_forwarder::forward_keypress(key, scancode);
}

glfw_gl_window::glfw_gl_window() {
    glfwSetErrorCallback(error_callback);

    if(!glfwInit()) {
        LOG(FATAL) << "Could not initialize GLFW";
    }

    init();
}

int glfw_gl_window::init() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(640, 480, "Minecraft Nova Renderer", NULL, NULL);
    if(!window) {
        LOG(FATAL) << "Could not initialize window :(";
    }

    LOG(INFO) << "GLFW window created";

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    if(!gladLoadGL()) {
        LOG(FATAL) << "Could not load OpenGL";
    }

    glfwGetFramebufferSize(window, &window_dimensions.x, &window_dimensions.y);

    glfwSetKeyCallback(window, key_callback);

    return 0;
}

glfw_gl_window::~glfw_gl_window() {
    glfwTerminate();
}

void glfw_gl_window::destroy() {
    glfwDestroyWindow(window);
    window = NULL;
}

void glfw_gl_window::set_fullscreen(bool fullscreen) {
    // TODO: RAVEN WRIT THIS
}

bool glfw_gl_window::should_close() {
    return (bool) glfwWindowShouldClose(window);
}

glm::vec2 glfw_gl_window::get_size() {
    return window_dimensions;
}

void glfw_gl_window::end_frame() {
    glfwSwapBuffers(window);
    glfwPollEvents();
}
