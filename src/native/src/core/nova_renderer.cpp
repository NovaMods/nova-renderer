//
// Created by David on 25-Dec-15.
//

#include "nova_renderer.h"

#include <easylogging++.h>

#include "../gl/windowing/glfw_gl_window.h"
#include "../utils/utils.h"

INITIALIZE_EASYLOGGINGPP

std::unique_ptr<nova_renderer> nova_renderer::instance;

nova_renderer::nova_renderer() : gui_renderer_instance(tex_manager, shader_manager, ubo_manager) {
    initialize_logging();

    // Oh wow this line is gross. I guess this is why everyone hates CPP?
    game_window = std::unique_ptr<iwindow>(static_cast<iwindow *>(new glfw_gl_window()));
    //game_window = make_unique<iwindow>(new glfw_gl_window());

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

nova_renderer::~nova_renderer() {
    game_window->destroy();
}

bool nova_renderer::has_render_available() {
    return false;
}

void nova_renderer::render_frame() {
    // Clear to the clear color
    // glClear(GL_COLOR_BUFFER_BIT);

    // TODO: Move this thing so that GUI rendering calls only get dispatched when the GUI has changed
    gui_renderer_instance.render();

    // Render solid geometry
    // Render entities
    // Render transparent things

    game_window->end_frame();
}

bool nova_renderer::should_end() {
    // If the window wants to close, the user probably clicked on the "X" button
    return game_window->should_close();
}

void nova_renderer::init_instance() {
    instance = std::unique_ptr<nova_renderer>(new nova_renderer());
}

texture_manager & nova_renderer::get_texture_manager() {
    return tex_manager;
}

shader_store &nova_renderer::get_shader_manager() {
    return shader_manager;
}

uniform_buffer_store &nova_renderer::get_ubo_manager() {
    return ubo_manager;
}

gui_renderer & nova_renderer::get_gui_renderer() {
    return gui_renderer_instance;
}






