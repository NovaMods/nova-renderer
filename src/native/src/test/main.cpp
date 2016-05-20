#include <cstdlib>
#include "shader_test.h"
#include "../utils/utils.h"
#include "sanity.h"
#include "../core/nova_renderer.h"

int main() {
    initialize_logging();
    // Construct the renderer so we have an OpenGL context to work with
    nova_renderer renderer;

    sanity::run_all();

    shader::run_all();

    system("PAUSE");

    return 0;
}