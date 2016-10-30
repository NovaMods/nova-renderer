/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_GUI_RENDERER_H
#define RENDERER_GUI_RENDERER_H

#include <memory>
#include <atomic>

#include <mutex>

#include "model/gl/gl_mesh.h"
#include "mc_interface/mc_gui_objects.h"

namespace nova {
    namespace model {
        /*!
         * \brief Constructs the geometry needed to render the current GUI screen
         *
         * Note that the GUI screen does not include things like the spinning background on the main menu screen, because
         * that's going to be rendered as if it was a scene
         */
        gl_mesh build_gui_geometry(mc_gui_screen &cur_screen);
    }
}

#endif //RENDERER_GUI_RENDERER_H
