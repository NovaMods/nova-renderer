#include <easylogging++.h>
#include "../core/nova_renderer.h"
#include "../gl/objects/gl_shader_program.h"

#include "sanity.h"
#include "shader_test.h"

int main() {
    // Open the window first, so we have an OpenGL context to play with
    nova_renderer nova;

    LOG(INFO) << "Running sanity tests...";
    sanity::run_all();

    LOG(INFO) << "Running shader tests...";
    shader::run_all();

    LOG(INFO) << "Integration tests...";

    gl_shader_program * gui_shader = new gl_shader_program();
    gui_shader->add_shader(GL_VERTEX_SHADER, "shaderpacks/default/shaders/gui.vert");
    gui_shader->add_shader(GL_FRAGMENT_SHADER, "shaderpacks/default/shaders/gui.frag");
    gui_shader->link();

    nova.get_shader_manager().emplace("gui", gui_shader);

    nova.get_ubo_manager().emplace("camera_data", new gl_uniform_buffer(sizeof(camera_data)));

    while(!nova.should_end()) {
        nova.render_frame();
    }

    return 0;
}