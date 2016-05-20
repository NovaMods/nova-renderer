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

/*!
 * \brief Defines a bunch of methods to render different GUI elements, including buttons, text, and images
 *
 * Something interesting to note - GUI layout is all calculated in pixels, then all the vertices are scaled down by the
 * view matrix.
 */
class gui_renderer {
public:
    gui_renderer(texture_manager * textures, shader_store * shaders);
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

    const std::string WIDGETS_TEXTURE_NAME = "textures/gui/widgets.png";
    const std::string GUI_SHADER_NAME = "gui";

    void create_unpressed_button();
    void create_pressed_button();
};


#endif //RENDERER_GUI_RENDERER_H
