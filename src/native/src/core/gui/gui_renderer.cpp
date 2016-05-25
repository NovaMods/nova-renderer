/*!
 * \author David
 * \date 13-May-16.
 */

#include "gui_renderer.h"
#include "../../gl/objects/gl_vertex_buffer.h"

gui_renderer::gui_renderer(texture_manager * textures, shader_store * shaders, uniform_buffer_manager * uniform_buffers) {
    create_unpressed_button();
    create_pressed_button();

    tex_manager = textures;
    shader_manager = shaders;
    this->ubo_manager = uniform_buffers;

    setup_camera_buffer();
}

void gui_renderer::setup_camera_buffer() const {
    // Update the GUI camera data buffer
    // TODO: Figure out camera data parameters
    // I should only need to do this once and use the same MVP matrix for every GUI render
    gl_uniform_buffer * buf = ubo_manager->get_buffer("guiCameraData");
    buf->bind();
    GLvoid * p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    memcpy(p, &cam_data.mvp[0][0], sizeof(glm::mat4));
    glUnmapBuffer(GL_UNIFORM_BUFFER);

    buf->set_bind_point(0);
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
    // Check the the new screen is different


    cur_screen = screen;

    // We can re-build the screen geometry here if we have a new screen.
    // We know if we have a new screen because MC GUI screens have IDs
    // If we do have the same screen, we can look at all the buttons to see if any of them have a different pressed
    // state
}

void gui_renderer::render() {
    /*
     * Render needs to do a few things
     *
     * First, bind the GUI shader. We'll need access to the shaders store
     *
     * We'll also need a way to draw items and whatnot
     */

    // Bind the GUI shader
    ishader * gui_shader = shader_manager->get_shader(GUI_SHADER_NAME);
    gui_shader->bind();

    // Bind the GUI buttons texture to texture unit 0
    itexture * gui_tex = tex_manager->get_texture_atlas(texture_manager::atlas_type::GUI, texture_manager::texture_type::ALBEDO);
    gui_tex->bind(GL_TEXTURE0);
    unpressed_button_buffer->draw();
}
