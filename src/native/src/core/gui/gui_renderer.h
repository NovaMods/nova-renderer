/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_GUI_RENDERER_H
#define RENDERER_GUI_RENDERER_H

#include <memory>
#include "../../interfaces/ivertex_buffer.h"
#include "../../mc/mc_gui_objects.h"
#include "../texture_manager.h"
#include "../shaders/uniform_buffers.h"
#include "../types.h"

/*!
 * \brief Defines a bunch of methods to render different GUI elements, including buttons, text, and images
 *
 * Something interesting to note - GUI layout is all calculated in pixels, then all the vertices are scaled down by the
 * view matrix.
 */
class gui_renderer {
public:
    gui_renderer(texture_manager & textures,
                 shader_store & shaders,
                 uniform_buffer_store & uniform_buffers);
    ~gui_renderer();

    /*!
     * \brief Sets up the uniform buffer for GUI geometry
     *
     * This needs to be a separate method. I can't use RAII because the uniform buffer needs OpenGL calls, which aren't
     * available until after I create a window, because coupling is weird.
     *
     * So this method needs to be called, but I can't enforce is being called. Whoops.
     */
    void setup_buffer();

    /*!
     * \brief Sets the GUI screen to render as the given screen
     */
    void set_current_screen(mc_gui_screen * screen);

    /*!
     * \brief Renders the current GUI screen
     *
     * Note: for right now, it just draws the unpressed button, just to prove that I can
     */
    void render();

    void setup_camera_buffer() const;

private:
    std::vector<GLfloat> unpressed_button_buffer = {
            0,   0,  0,     0,       0.3359375,
            200, 0,  0,     0.78125, 0.3359375,
            0,   20, 0,     0,       0.4156963,
            200, 20, 0,     0.78125, 0.4156963
    };

    std::vector<GLfloat> pressed_button_buffer = {
            0,   0,  0,     0,       0.2578112,
            200, 0,  0,     0.78125, 0.2578112,
            0,   20, 0,     0,       0.3359375,
            200, 20, 0,     0.78125, 0.3359375
    };

    std::vector<GLshort> index_buffer = {
            0, 1, 2,
            2, 3, 0
    };

    mc_gui_screen * cur_screen;

    texture_manager & tex_manager;
    shader_store & shader_manager;
    uniform_buffer_store & ubo_manager;

    gui_camera_data cam_data;

    std::string WIDGETS_TEXTURE_NAME = "textures/gui/widgets.png";
    std::string GUI_SHADER_NAME = "gui";

    std::unique_ptr<ivertex_buffer> cur_screen_buffer;

    /*!
     * \brief Compares two mc_gui_screen objects, determining if they represent the same visual data
     *
     * I'l like to have had this function in the same header file as the mc_gui_screen struct. However, I need
     * mc_gui_screen to be a C struct so I can properly assign to it from Java. The compiler yelled at me about "You
     * can't compare structs" so I couldn't use the == operator and here we are.
     */
    bool is_same_screen(mc_gui_screen *screen1, mc_gui_screen *screen2);

    bool same_buttons(mc_gui_button button1, mc_gui_button button2);

    void build_gui_geometry();
};


#endif //RENDERER_GUI_RENDERER_H
