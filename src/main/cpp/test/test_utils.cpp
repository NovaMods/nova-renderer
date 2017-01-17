/*!
 * \brief
 *
 * \author ddubois 
 * \date 17-Jan-17.
 */

#include "test_utils.h"

mc_gui_screen get_gui_screen_one_button()  {
    mc_gui_screen screen = {};
    screen.num_buttons = 1;

    mc_gui_button button;
    button.x_position = 0;
    button.y_position = 0;
    button.width = 100;
    button.height = 100;
    button.text = "Default text";
    button.is_pressed = false;

    screen.buttons[0] = button;

    return screen;
}
