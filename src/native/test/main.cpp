#include <iostream>
#include <memory.h>
#include <easylogging++.h>
#include "../src/core/nova.h"

using namespace std;

INITIALIZE_EASYLOGGINGPP

void initialize_logging() {
    // Configure the logger
    el::Configurations conf("config/logging.conf");

    // Turn debug and trace off in release builds
#ifdef NDEBUG
    conf.parseFromText("*DEBUG:\n ENABLED=false");
    conf.parseFromText("*TRACE:\n ENABLED=false");
#else
    conf.parseFromText("*ALL: FORMAT = \"%datetime{%h:%m:%s} [%level] at %loc - %msg\"");
#endif

    el::Loggers::reconfigureAllLoggers(conf);
}

int main() {
    initialize_logging();
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

    while(!should_close()) {
        do_test_render();
    }

    return 0;
}