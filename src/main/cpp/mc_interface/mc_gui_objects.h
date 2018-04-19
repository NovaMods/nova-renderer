/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_MC_GUI_OBJECTS_H
#define RENDERER_MC_GUI_OBJECTS_H

#define MAX_NUM_BUTTONS 22

/*!
 * \brief Represents a single button in the GUI
 *
 * Some buttons don't have text, they have pictures. Those pictures are just images. I can have no text and just make
 * the UVs point to the proper image
 *
 * Except that the vertex buffers are set up so that they always point to the same texture. I'll figure this out at
 * some point.
 */
struct mc_gui_button {
    int x_position;
    int y_position;
    int width;
    int height;
    const char * text;
    bool is_pressed;
	int enabled;
};

/*!
 * \brief Represents a screen in the GUI
 *
 * This is the same concept as MC's gui screens. There should be exactly one of these active at a given time.
 *
 * There's almost certainly a better way to do this
 */
struct mc_gui_screen {
    mc_gui_button buttons[MAX_NUM_BUTTONS];
    int num_buttons;
};

#endif //RENDERER_MC_GUI_OBJECTS_H
