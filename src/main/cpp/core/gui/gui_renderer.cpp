/*!
 * \author David
 * \date 13-May-16.
 */

#include <algorithm>
#include <easylogging++.h>
#include "gui_renderer.h"
#include "model/gl/gl_vertex_buffer.h"

gui_renderer::gui_renderer(nova::model::texture_manager & textures, nova::model::shaderpack & shaders, nova::model::uniform_buffer_store & uniform_buffers) :
        tex_manager(textures), shaders(shaders), ubo_manager(uniform_buffers), has_screen_available(false) {
    LOG(INFO) << "Created GUI Renderer";

    cur_screen = {};

    setup_buffers();
}

gui_renderer::~gui_renderer() {
}

void gui_renderer::set_current_screen(mc_gui_screen *screen) {
    new_screen = *screen;
    has_screen_available = true;
}

void gui_renderer::render() {
    // Bind the GUI shader
    nova::model::gl_shader_program & gui_shader = shaders.get_shader(GUI_SHADER_NAME);
    gui_shader.bind();

    // Bind the GUI buttons texture to texture unit 0
    // Commented out because we don't support textures yet. Not really.
    //texture2D& gui_tex = tex_manager.get_texture_atlas(texture_manager::atlas_type::GUI, texture_manager::texture_type::ALBEDO);
    //gui_tex.bind(0);

    // Draw the 2D GUI geometry
    cur_screen_buffer->set_active();
    cur_screen_buffer->draw();
}

bool gui_renderer::is_different_screen(mc_gui_screen &screen1, mc_gui_screen &screen2) const {
    for(int i = 0; i < MAX_NUM_BUTTONS; i++) {
        if(different_buttons(screen1.buttons[i], screen2.buttons[i])) {
            return true;
        }
    }

    return false;
}

bool gui_renderer::different_buttons(mc_gui_button &button1, mc_gui_button &button2) const {
    bool same_rect = button1.x_position == button2.x_position &&
            button1.y_position == button2.y_position &&
            button1.width == button2.width &&
            button1.height == button2.height;

    bool same_text = compare_text(button1.text, button2.text);

    bool same_pressed = button1.is_pressed == button2.is_pressed;

    return !same_rect || !same_text || !same_pressed;
}

void gui_renderer::build_gui_geometry() {
    // We need to make a vertex buffer with the positions and texture coordinates of all the gui elements
    std::vector<float> vertex_buffer(MAX_NUM_BUTTONS * 4 * 5);    // MAX_NUM_BUTTONS buttons * 4 vertices per button * 5 elements per vertex
    vertex_buffer.clear();
    std::vector<unsigned short> indices;
    unsigned short start_pos = 0;

    for(int i = 0; i < cur_screen.num_buttons; i++) {
        mc_gui_button & button = cur_screen.buttons[i];

        // TODO: More switches to figure out exactly which UVs we should use
        std::vector<float> & uv_buffer = basic_unpressed_uvs;
        if(button.is_pressed) {
            uv_buffer = basic_pressed_uvs;
        }

        // Generate the vertexes from the button's position
        add_vertices_from_button(vertex_buffer, button, uv_buffer);

        add_indices_for_button(indices, start_pos);

        // Add the number of new vertices to the offset for indices, so that indices point to the right
        // vertices
        start_pos += 4;
    }

    cur_screen_buffer->set_data(vertex_buffer, ivertex_buffer::format::POS_UV, ivertex_buffer::usage::static_draw);
    cur_screen_buffer->set_index_array(indices, ivertex_buffer::usage::static_draw);
}

void gui_renderer::add_indices_for_button(std::vector<unsigned short> &indices, unsigned short start_pos) {
    for(unsigned short &index : index_buffer) {
        indices.push_back(index + start_pos);
    }
}

void gui_renderer::add_vertices_from_button(std::vector<GLfloat> &vertex_buffer, const mc_gui_button &button,
                                            const std::vector<GLfloat> &uvs) {
    add_vertex(
            vertex_buffer,
            button.x_position, button.y_position,
            uvs[0], uvs[1]
    );
    add_vertex(
            vertex_buffer,
            button.x_position + button.width, button.y_position,
            uvs[2], uvs[3]
    );
    add_vertex(
            vertex_buffer,
            button.x_position, button.y_position + button.height,
            uvs[4], uvs[5]
    );
    add_vertex(
            vertex_buffer,
            button.x_position + button.width, button.y_position + button.height,
            uvs[6], uvs[7]
    );
}

void gui_renderer::setup_buffers() {
    // Buffer for the GUI geometry
    cur_screen_buffer = std::unique_ptr<ivertex_buffer>(new nova::model::gl_vertex_buffer());
}

void gui_renderer::add_vertex(std::vector<float> &vertex_buffer, int x, int y, float u, float v) {
    vertex_buffer.push_back(static_cast<float>(x));
    vertex_buffer.push_back(static_cast<float>(y));
    vertex_buffer.push_back(0.0f);

    vertex_buffer.push_back(u);
    vertex_buffer.push_back(v);
}

void gui_renderer::update() {
    if(has_screen_available) {
        has_screen_available = false;

        // We want to spend as little time as possible with locks on
        new_screen_guard.lock();
        mc_gui_screen new_screen_copy = new_screen;
        new_screen_guard.unlock();

        bool is_different = is_different_screen(cur_screen, new_screen_copy);

        if(is_different) {
            LOG(INFO) << "Switching to a new GUI screen";
            cur_screen = new_screen_copy;
            build_gui_geometry();
        }
    }
}

bool gui_renderer::compare_text(const char *text1, const char *text2) const {
    if(text1 == nullptr && text2 == nullptr) {
        // They're both null, and null equals null, so they're the same
        // If this causes problems I'll change it
        return true;
    }

    if(text1 == nullptr) {
        return false;
    }

    if(text2 == nullptr) {
        return false;
    }

    return strcmp(text1, text2) == 0;
}




