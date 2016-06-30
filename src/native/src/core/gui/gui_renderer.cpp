/*!
 * \author David
 * \date 13-May-16.
 */

#include <algorithm>
#include <easylogging++.h>
#include "gui_renderer.h"
#include "../../gl/objects/gl_vertex_buffer.h"

gui_renderer::gui_renderer(texture_manager & textures,
                           shaderpack & shaders,
                           uniform_buffer_store & uniform_buffers) :
        tex_manager(textures),
        shaders(shaders),
        ubo_manager(uniform_buffers) {
    LOG(INFO) << "Created GUI Renderer";
}

gui_renderer::~gui_renderer() {
}

void gui_renderer::set_current_screen(mc_gui_screen *screen) {
    // Check the the new screen is different
    if(cur_screen != NULL && is_same_screen(*cur_screen, *screen)) {
        return;
    }

    cur_screen = screen;

    build_gui_geometry();
    LOG(INFO) << "GUI geometry built successfully";
}

void gui_renderer::render() {
    // Bind the GUI shader
    gl_shader_program & gui_shader = shaders.get_shader(GUI_SHADER_NAME);
    gui_shader.bind();

    // Bind the GUI buttons texture to texture unit 0
    texture2D & gui_tex = tex_manager.get_texture_atlas(texture_manager::atlas_type::GUI, texture_manager::texture_type::ALBEDO);
    gui_tex.bind(GL_TEXTURE0);

    // Draw the 2D GUI geometry
    cur_screen_buffer->set_active();
    cur_screen_buffer->draw();
}

bool gui_renderer::is_same_screen(mc_gui_screen &screen1, mc_gui_screen &screen2) const {
    for(int i = 0; i < MAX_NUM_BUTTONS; i++) {
        if(same_buttons(screen1.buttons[i], screen2.buttons[i])) {
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
    std::vector<float> vertex_buffer(MAX_NUM_BUTTONS * 4 * 5);    // MAX_NUM_BUTTONS buttons * 4 vertices per button * 5 elements per vertex
    vertex_buffer.clear();
    std::vector<unsigned short> indices;
    unsigned short start_pos = 0;

    for(int i = 0; i < cur_screen->num_buttons; i++) {
        mc_gui_button & button = cur_screen->buttons[i];

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
    cur_screen_buffer = std::unique_ptr<ivertex_buffer>(new gl_vertex_buffer());
}

void gui_renderer::do_init_tasks() {
    setup_buffers();
}

void gui_renderer::add_vertex(std::vector<float> &vertex_buffer, int x, int y, float u, float v) {
    vertex_buffer.push_back(x);
    vertex_buffer.push_back(y);
    vertex_buffer.push_back(0);

    vertex_buffer.push_back(u);
    vertex_buffer.push_back(v);
}


