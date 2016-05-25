/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_MC_GUI_OBJECTS_H
#define RENDERER_MC_GUI_OBJECTS_H

#define MAX_NUM_BUTTONS 22

struct mc_gui_button {
    int x_position;
    int y_position;
    int width;
    int height;
    const char * text;
    bool is_pressed;
};

struct mc_gui_screen {
    int screen_id;
    mc_gui_button buttons[MAX_NUM_BUTTONS];
};

#endif //RENDERER_MC_GUI_OBJECTS_H
