#include <thread>
#include <chrono>

#include <unistd.h>

#include "core/nova.h"

void fill_render_command(mc_render_command &command);

int main() {
    // Open the window first, so we have an OpenGL context to play with
    init_nova();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    //usleep(5000000);

    // Build a basic GUI thing
    mc_set_gui_screen_command gui_command;
    gui_command.screen.buttons[0].width = 20;
    gui_command.screen.buttons[0].height = 20;
    gui_command.screen.buttons[0].x_position = 0;
    gui_command.screen.buttons[0].y_position = 0;
    gui_command.screen.buttons[0].is_pressed = false;
    gui_command.screen.buttons[0].text = "";

    gui_command.screen.num_buttons = 1;

    send_change_gui_screen_command(&gui_command);

    // This needs to run forever so we don't exit
    while(!should_close() || true) {
        // Make a dummy render command
        mc_render_command command;
        fill_render_command(command);

        send_render_command(&command);
    }

    return 0;
}

void fill_render_command(mc_render_command &command) {
    command.mouse_x = 0;
    command.mouse_y = 0;
    command.previous_frame_time = 0;
    command.render_world_params.camera_x = 0;
    command.render_world_params.camera_y = 0;
    command.render_world_params.camera_z = 0;
}
