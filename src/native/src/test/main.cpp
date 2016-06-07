#include "../core/nova_renderer.h"
#include "../gl/objects/gl_shader_program.h"

int main() {
    nova_renderer nova;

    gl_shader_program * gui_shader = new gl_shader_program();
    gui_shader->add_shader(GL_VERTEX_SHADER, "shaderpacks/default/shaders/gui.vert");
    gui_shader->add_shader(GL_FRAGMENT_SHADER, "shaderpacks/default/shaders/gui.frag");
    gui_shader->link();

    nova.get_shader_manager().emplace("gui", gui_shader);

    nova.get_ubo_manager().emplace("camera_data", new gl_uniform_buffer(sizeof(camera_data)));

    nova.get_gui_renderer().setup_buffer();

    while(!nova.should_end()) {
        nova.render_frame();
    }

    return 0;
}