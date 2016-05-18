/*!
 * \author David
 * \date 13-May-16.
 */

#include "gui_renderer.h"
#include "../../gl/objects/gl_vertex_buffer.h"

gui_renderer::gui_renderer(texture_manager * tex_manager) {
    create_unpressed_button();
    create_pressed_button();

    this->tex_manager = tex_manager;
}

gui_renderer::~gui_renderer() {
}

void gui_renderer::create_unpressed_button() {
    unpressed_button_buffer = new gl_vertex_buffer();

    std::vector<float> unpressed_data = {
            0,   0,  0,     0,       0.2578112,
            200, 0,  0,     0.78125, 0.2578112,
            0,   20, 0,     0,       0.3359375,
            200, 20, 0,     0.78125, 0.3359375
    };

    unpressed_button_buffer->set_data(unpressed_data, ivertex_buffer::format::POS_UV, ivertex_buffer::usage::static_draw);
}

void gui_renderer::create_pressed_button() {
    pressed_button_buffer = new gl_vertex_buffer();

    std::vector<float> pressed_data = {
            0,   0,  0,     0,       0.3359375,
            200, 0,  0,     0.78125, 0.3359375,
            0,   20, 0,     0,       0.4156963,
            200, 20, 0,     0.78125, 0.4156963
    };

    pressed_button_buffer->set_data(pressed_data, ivertex_buffer::format::POS_UV, ivertex_buffer::usage::static_draw);
}

void gui_renderer::set_current_screen(mc_gui_screen *screen) {
    cur_screen = screen;
}

void gui_renderer::render() {
    /*
     * Render needs to do a few things
     *
     * First, bind the GUI shader. We'll need access to the shaders store
     *
     * We'll also need a way to draw items and whatnot
     */

    // Bind the GUI buttons texture to texture unit 0

    itexture * gui_tex = tex_manager->get_texture_atlas(texture_manager::atlas_type::GUI, texture_manager::texture_type::ALBEDO);
    gui_tex->bind(GL_TEXTURE0);
    unpressed_button_buffer->draw();
}
