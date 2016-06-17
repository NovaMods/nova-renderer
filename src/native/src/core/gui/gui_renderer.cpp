/*!
 * \author David
 * \date 13-May-16.
 */

#include <algorithm>
#include <easylogging++.h>
#include "gui_renderer.h"
#include "../../gl/objects/gl_vertex_buffer.h"

gui_renderer::gui_renderer(texture_manager & textures,
                           shader_store & shaders,
                           uniform_buffer_store & uniform_buffers) :
        tex_manager(textures),
        shader_manager(shaders),
        ubo_manager(uniform_buffers) {
    LOG(INFO) << "Creating GUI Renderer";
}

gui_renderer::~gui_renderer() {
}

void gui_renderer::set_current_screen(mc_gui_screen *screen) {
    // Check the the new screen is different
    if(is_same_screen(cur_screen, screen)) {
        return;
    }

    cur_screen = screen;

    build_gui_geometry();
}

void gui_renderer::render() {
    LOG(TRACE) << "Rendering the GUI";

    // Bind the GUI shader
    ishader * gui_shader = shader_manager[GUI_SHADER_NAME];
    gui_shader->bind();

    // Bind the GUI buttons texture to texture unit 0
    itexture * gui_tex = tex_manager.get_texture_atlas(texture_manager::atlas_type::GUI, texture_manager::texture_type::ALBEDO);
    gui_tex->bind(GL_TEXTURE0);

    // Draw the 2D GUI geometry
    cur_screen_buffer->draw();
}

bool gui_renderer::is_same_screen(mc_gui_screen *screen1, mc_gui_screen *screen2) const {
    if(screen1 == NULL && screen2 == NULL) {
        return true;
    } else if(screen1 == NULL) {
        return false;
    } else if(screen2 == NULL) {
        return false;   // TODO: Determine is thie is absolutely the best way to handle null screens
    }


    if(screen1->screen_id != screen2->screen_id) {
        return false;
    }

    for(int i = 0; i < MAX_NUM_BUTTONS; i++) {
        if(same_buttons(screen1->buttons[i], screen2->buttons[i])) {
            return false;
        }
    }

    return true;
}

bool gui_renderer::same_buttons(mc_gui_button & button1, mc_gui_button & button2) const {
    return button1.x_position == button2.x_position &&
            button1.y_position == button2.y_position &&
            button1.width == button2.width &&
            button1.height == button2.height &&
            strcmp(button1.text, button2.text) == 0 &&
            button1.is_pressed == button2.is_pressed;
}

void gui_renderer::build_gui_geometry() {
    // We need to make a vertex buffer with the positions and texture coordinates of all the gui elements
    std::vector<GLfloat> buffer(MAX_NUM_BUTTONS * 4 * 5);    // MAX_NUM_BUTTONS buttons * 4 vertices per button * 5 elements per vertex
    std::vector<GLshort> indices;

    std::for_each(
            std::begin(cur_screen->buttons),
            std::end(cur_screen->buttons),
            [&](mc_gui_button & button){
                short start_pos = (short) (buffer.size() - 1);

                if(button.is_pressed) {
                    buffer.insert(pressed_button_buffer.begin(), pressed_button_buffer.end(), buffer.end());
                } else {
                    buffer.insert(unpressed_button_buffer.begin(), unpressed_button_buffer.end(), buffer.end());
                }

                std::vector<GLshort> indices_to_add(6);
                std::transform(index_buffer.begin(), index_buffer.end(), indices_to_add.begin(), [=](GLushort & num) {return num + start_pos;});

                indices.insert(indices_to_add.begin(), indices_to_add.end(), indices.end());
            });
}

void gui_renderer::setup_buffer() {
    cur_screen_buffer = std::unique_ptr<ivertex_buffer>(new gl_vertex_buffer());
}

void gui_renderer::build_default_gui() {
    setup_buffer();
    cur_screen_buffer->set_data(unpressed_button_buffer, ivertex_buffer::format::POS_UV, ivertex_buffer::usage::static_draw);
    cur_screen_buffer->set_index_array(index_buffer, ivertex_buffer::usage::static_draw);
}
