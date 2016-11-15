/*!
 * \brief
 *
 * \author ddubois 
 * \date 14-Nov-16.
 */

#include "mesh_builder.h"
#include "gui_geometry_builder.h"

namespace nova {
    namespace model {
        void mesh_builder::build_geometry(mc_gui_screen &screen) {
            mesh_definition gui_object = build_gui_geometry(screen);

            gui_mesh_lock.lock();
            has_new_gui = true;
            gui_mesh = gui_object;
            gui_mesh_lock.lock();
        }

        mesh_definition mesh_builder::get_gui_mesh() {
            gui_mesh_lock.lock();
            has_new_gui = false;
            mesh_definition ret_val = std::move(gui_mesh);
            gui_mesh_lock.unlock();

            return ret_val;
        }

        const bool mesh_builder::has_new_gui_mesh() const {
            return has_new_gui;
        }
    }
}
