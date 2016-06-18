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

nova_renderer::nova_renderer() : gui_renderer_instance(tex_manager, shader_manager, ubo_manager) {
    initialize_logging();

    // Oh wow this line is gross. I guess this is why everyone hates CPP?
    game_window = std::unique_ptr<iwindow>(new glfw_gl_window());

    gui_renderer_instance.build_default_gui();

    // Here's a bunch of really gross code to Make It Work (TM)
    triangle = std::unique_ptr<ivertex_buffer>(new gl_vertex_buffer());

    static const std::vector<GLfloat> vertex_buffer_data = {
            -1.0f, -1.0f, 0.0f,     1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,      0.0f, 1.0f,
            0.0f, 1.0f, 0.0f,       0.0f, 0.0f
    };

    triangle->set_data(vertex_buffer_data, ivertex_buffer::format::POS_UV, ivertex_buffer::usage::static_draw);

    std::vector<unsigned short> indices = {0, 1, 2};

    triangle->set_index_array(indices, ivertex_buffer::usage::static_draw);
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

    // TODO: Move this thing so that GUI rendering calls only get dispatched when the GUI has changed
    //gui_renderer_instance.render();

    shader_manager["triangle"]->bind();

    // More gross code just to Make It Work (TM)
    triangle->set_active();
    triangle->draw();

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

shader_store &nova_renderer::get_shader_manager() {
    return shader_manager;
}

uniform_buffer_store &nova_renderer::get_ubo_manager() {
    return ubo_manager;
}

gui_renderer & nova_renderer::get_gui_renderer() {
   return gui_renderer_instance;
}






