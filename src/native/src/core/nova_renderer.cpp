//
// Created by David on 25-Dec-15.
//

#include "nova_renderer.h"

#include <easylogging++.h>

#include "../gl/windowing/glfw_gl_window.h"


INITIALIZE_EASYLOGGINGPP

void initialize_logging() {
    // Configure the logger
    el::Configurations conf("config/logging.conf");

    // Turn debug and trace off in release builds
#ifdef NDEBUG
    conf.parseFromText("*DEBUG:\n ENABLED=false");
    conf.parseFromText("*TRACE:\n ENABLED=false");
#endif

    el::Loggers::reconfigureAllLoggers(conf);
}

nova_renderer * nova_renderer::instance;

nova_renderer::nova_renderer() {
    initialize_logging();

    m_game_window = new glfw_gl_window();
    LOG(INFO) << "Instantiated window";
}

nova_renderer::~nova_renderer() {
    m_game_window->destroy();
}

bool nova_renderer::has_render_available() {
    return false;
}

void nova_renderer::render_frame() {
    m_game_window->end_frame();
}

bool nova_renderer::should_continue() {
    // If the window wants to close, the user probably clicked on the "X" button
    return m_game_window->should_close();
}

void nova_renderer::init_instance() {
    instance = new nova_renderer();
}

nova_renderer *nova_renderer::get_instance() {
    return instance;
}

void nova_renderer::add_texture(mc_texture &new_texture) {
    num_textures++;
    LOG(INFO) << num_textures << " textures loaded by Nova";
}






