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

            gui_mesh.set_resource(gui_object);
        }

        loaded_resource<mesh_definition>& mesh_builder::get_gui_mesh() {
            return gui_mesh;
        }
    }
}
