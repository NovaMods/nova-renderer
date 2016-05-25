/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_GUI_RENDERER_H
#define RENDERER_GUI_RENDERER_H

#include "../../interfaces/ivertex_buffer.h"
#include "../../mc/mc_gui_objects.h"
#include "../texture_manager.h"
#include "../shaders/shader_store.h"
#include "../shaders/uniform_buffer_manager.h"
#include "../shaders/uniform_buffers.h"

/*!
 * \brief Defines a bunch of methods to render different GUI elements, including buttons, text, and images
 *
 * Something interesting to note - GUI layout is all calculated in pixels, then all the vertices are scaled down by the
 * view matrix.
 */
class gui_renderer {
public:
    gui_renderer(texture_manager * textures, shader_store * shaders, uniform_buffer_manager * uniform_buffers);
    ~gui_renderer();

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

private:
    ivertex_buffer * unpressed_button_buffer;
    ivertex_buffer * pressed_button_buffer;

    mc_gui_screen * cur_screen;

    texture_manager * tex_manager;
    shader_store * shader_manager;
    uniform_buffer_manager * ubo_manager;

    guiCameraData cam_data;

    std::string WIDGETS_TEXTURE_NAME = "textures/gui/widgets.png";
    std::string GUI_SHADER_NAME = "gui";

    void create_unpressed_button();
    void create_pressed_button();

    void setup_camera_buffer() const;

    /*!
     * \brief Compares two mc_gui_screen objects, determining if they represnet the same visual data
     *
     * I'l like to have had this function in the same header file as the mc_gui_screen struct. However, I need
     * mc_gui_screen to be a C struct so I can properly assign to it from Java. The compiler yelled at me about "You
     * can't compare structs" so I couldn't use the == operator and here we are.
     */
    bool same_screen(mc_gui_screen *screen1, mc_gui_screen *screen2);

    bool same_buttons(mc_gui_button button1, mc_gui_button button2);
};


#endif //RENDERER_GUI_RENDERER_H
