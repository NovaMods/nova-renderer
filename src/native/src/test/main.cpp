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

    while(!nova.should_end()) {
        nova.render_frame();
    }

    return 0;
}