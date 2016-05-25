#include "../core/nova_renderer.h"

int main() {
    nova_renderer nova;

    while(!nova.should_end()) {
        nova.render_frame();
    }

    return 0;
}