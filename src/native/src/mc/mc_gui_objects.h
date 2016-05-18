/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_MC_GUI_OBJECTS_H
#define RENDERER_MC_GUI_OBJECTS_H

class mc_gui_button {
    int x_position;
    int y_position;
    int width;
    int height;
    const char * text;
};

struct mc_gui_screen {
    mc_gui_button buttons[22];
};

#endif //RENDERER_MC_GUI_OBJECTS_H
