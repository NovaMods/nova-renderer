/*!
 * \brief
 *
 * \author ddubois 
 * \date 14-Nov-16.
 */

#include <easylogging++.h>
#include "mesh_builder.h"
#include "gui_geometry_builder.h"

namespace nova {
    void mesh_builder::build_geometry(mc_gui_screen &screen) {
        bool different = are_different_screens(screen, cur_gui_screen);
        if(different) {
            cur_gui_screen = screen;
            gui_mesh = build_gui_geometry(screen);
        }

    }

    mesh_definition &mesh_builder::get_gui_mesh() {
        return gui_mesh;
    }

    bool are_different_screens(const mc_gui_screen &screen1, const mc_gui_screen &screen2) {
        if(screen1.num_buttons != screen2.num_buttons) {
            return true;
        }

        for(int i = 0; i < MAX_NUM_BUTTONS; i++) {
            if(i >= screen1.num_buttons || i >= screen2.num_buttons) {
                break;
            }
            LOG(TRACE) << "Checking button " << i << " for similarity";
            if(are_different_buttons(screen1.buttons[i], screen2.buttons[i])) {
                LOG(TRACE) << "Button " << i << " is different";
                return true;
            }

            LOG(TRACE) << "Button " << i << " is the same in both screens";
        }

        return false;
    }

    bool are_different_buttons(const mc_gui_button &button1, const mc_gui_button &button2) {
        bool same_rect = button1.x_position == button2.x_position &&
                         button1.y_position == button2.y_position &&
                         button1.width == button2.width &&
                         button1.height == button2.height;

        bool same_text = !are_different_strings(button1.text, button2.text);

        bool same_pressed = button1.is_pressed == button2.is_pressed;

        return !same_rect || !same_text || !same_pressed;
    }

    bool are_different_strings(const char *text1, const char *text2) {
        if(text1 == nullptr && text2 == nullptr) {
            // They're both null, and null equals null, so they're the same
            // If this causes problems I'll change it
            return false;
        }

        if(text1 == nullptr) {
            return true;
        }

        if(text2 == nullptr) {
            return true;
        }

        return strcmp(text1, text2) != 0;
    }
}
