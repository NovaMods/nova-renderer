/*!
 * \brief
 *
 * \author ddubois 
 * \date 27-Sep-16.
 */

#include <stdlib.h>
#include "mesh_accessor.h"

#include "model/geometry_cache/builders/gui_geometry_builder.h"

namespace nova {
    namespace model {
        std::vector<render_object*> mesh_accessor::get_meshes_for_filter(std::function<bool(const render_object&)> &filter) {
            std::vector<render_object*> meshes(renderable_objects.size());

            render_objects_lock.lock();
            for(auto& renderable_object : renderable_objects) {
                if(filter(renderable_object)) { // here
                    meshes.push_back(&renderable_object);
                }
            }
            render_objects_lock.unlock();

            return meshes;
        }

        void mesh_accessor::add_render_object(const render_object& new_obj) {
            render_objects_lock.lock();

            renderable_objects.push_back(new_obj);

            render_objects_lock.unlock();
        }

        void mesh_accessor::build_geometry(mc_gui_screen &screen) {
            gl_mesh gui_object = build_gui_geometry(screen);

            // This is a hack. I'm sorry. I just want to Make It Work before refactoring the RAII away from OpenGL
            // objects
            // TODO: Fix this
            render_object gui = {};
            gui.geometry = (gl_mesh*)malloc(sizeof(gl_mesh));
            *gui.geometry = std::move(gui_object);
            gui.type = geometry_type::gui;
            gui.name = "gui";

            add_render_object(gui);
        }
    }
}
