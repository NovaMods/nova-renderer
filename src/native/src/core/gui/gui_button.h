/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_GUI_BUTTON_H
#define RENDERER_GUI_BUTTON_H

#include <GL/glm/glm.hpp>
#include "../../interfaces/ivertex_buffer.h"
#include "../../mc/mc_gui_objects.h"

class gui_button {
public:
    /*!
     * \brief Creates this GUI button from the given mc_gui_button
     *
     * \param mc_button The gui button to
     */
    gui_button(mc_gui_button & mc_button);

    /*!
     * \brief Checks if this button contains the given point.
     *
     * Useful for figuring out if this button should use the pressed or unpressed texture
     *
     * \param point The point to check if is inside this button
     *
     * \return True if the given point is within the button, false otherwise
     */
    bool contains_point(const glm::ivec2 & point) const;

    /*!
     * \brief Sets this buttons's vertex buffer to the given buffer
     *
     * \prarm buffer This button's vertex buffer
     */
    void give_vertex_buffer(const ivertex_buffer & buffer);

private:
    mc_gui_button data;
    ivertex_buffer * buffer;
};


#endif //RENDERER_GUI_BUTTON_H
