//
// Created by David on 25-Dec-15.
//

#include "nova_renderer.h"

#include <easylogging++.h>

#include "../gl/windowing/glfw_gl_window.h"
#include "../utils/utils.h"
#include "../gl/objects/gl_vertex_buffer.h"

INITIALIZE_EASYLOGGINGPP

std::unique_ptr<nova_renderer> nova_renderer::instance;

nova_renderer::nova_renderer() : game_window(new glfw_gl_window()),
                                 gui_renderer_instance(tex_manager, shaders, ubo_manager) {

    nova_config = std::unique_ptr<config>(new config("config/config.json"));
    nova_config->register_change_listener(game_window);
    nova_config->register_change_listener(&shaders);
    nova_config->update_change_listeners();

    gui_renderer_instance.do_init_tasks();

    create_ubos();
}

nova_renderer::~nova_renderer() {
    game_window->destroy();
}

bool nova_renderer::has_render_available() {
    return false;
}

void nova_renderer::render_frame() {
    // Clear to the clear color
    glClear(GL_COLOR_BUFFER_BIT);

    // Render GUI to GUI buffer
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

shaderpack &nova_renderer::get_shaderpack() {
    return shaders;
}

uniform_buffer_store &nova_renderer::get_ubo_manager() {
    return ubo_manager;
}

gui_renderer & nova_renderer::get_gui_renderer() {
   return gui_renderer_instance;
}

void nova_renderer::create_ubos() {
    // Build all the UBOs

    ubo_manager.emplace("cameraData", new gl_uniform_buffer(sizeof(camera_data)));

    // TODO: Make a config file from UBO name to bind point
}
