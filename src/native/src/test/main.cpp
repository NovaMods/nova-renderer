#include <iostream>
#include <memory.h>
#include "../core/nova.h"

using namespace std;

int main() {
    init_nova();
    cout << "Nova Renderer initialized" << endl;

    reset_texture_manager();

    mc_atlas_texture tex;
    tex.height = 16;
    tex.width = 16;
    tex.num_components = 4;
    tex.texture_data = (unsigned char *) new char[16 * 16 * 4];
    memset(tex.texture_data, 0, 16 * 16 * 4);

    add_texture(tex, 0, 0);
    cout << "Added test texture" << endl;

    return 0;
}