//
// Created by David on 25-Dec-15.
//

#include "nova_renderer.h"

#include <easylogging++.h>

#include "../gl/windowing/glfw_gl_window.h"
#include "../utils/utils.h"


INITIALIZE_EASYLOGGINGPP

nova_renderer * nova_renderer::instance;

nova_renderer::nova_renderer() {
    initialize_logging();

    m_game_window = new glfw_gl_window();
    LOG(INFO) << "Instantiated window";

    shader_manager = new shader_store();
    ubo_manager = new uniform_buffer_manager();

    gui_renderer_instance = new gui_renderer(&tex_manager, shader_manager, ubo_manager);

    glClearColor(1.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

nova_renderer::~nova_renderer() {
    m_game_window->destroy();
}

bool nova_renderer::has_render_available() {
    return false;
}

void nova_renderer::render_frame() {
    // Clear to the clear color
    //glClear(GL_COLOR_BUFFER_BIT);

    // TODO: Move this thing to that GUI rendering calls only get dispatched when the GUI has changed
    gui_renderer_instance->render();

    // Render solid geometry
    // Render entities
    // Render transparent things

    m_game_window->end_frame();
}

bool nova_renderer::should_end() {
    // If the window wants to close, the user probably clicked on the "X" button
    return m_game_window->should_close();
}

void nova_renderer::init_instance() {
    instance = new nova_renderer();
}

texture_manager *nova_renderer::get_texture_manager() {
    return &tex_manager;
}
