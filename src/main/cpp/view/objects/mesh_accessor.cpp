/*!
 * \brief
 *
 * \author ddubois 
 * \date 27-Sep-16.
 */

#include <algorithm>
#include <easylogging++.h>
#include "mesh_accessor.h"

namespace nova {
    mesh_accessor::mesh_accessor(mesh_builder &builder_ref) : mesh_builder(builder_ref) {}

    std::vector<render_object*> mesh_accessor::get_meshes_for_filter(std::function<bool(const render_object&)> &filter) {
        std::vector<render_object*> meshes;
        meshes.reserve(renderable_objects.size());

        for(auto& renderable_object : renderable_objects) {
            if(filter(renderable_object)) { // TODO: Crashes at this line. Re-think filters?
                meshes.push_back(&renderable_object);
            }
        }

        return meshes;
    }

    void mesh_accessor::update() {
        update_gui_mesh();
    }

    void mesh_accessor::update_gui_mesh() {
        loaded_resource<mesh_definition> gui_mesh = mesh_builder.get_gui_mesh();
        if(gui_mesh.is_new()) {
            auto gui_definition = gui_mesh.get_resource();

            render_object gui = {};
            gui.geometry = new gl_mesh(gui_definition);
            gui.type = geometry_type::gui;
            gui.name = "gui";

            auto old_gui_object = std::find_if(renderable_objects.begin(), renderable_objects.end(),
                                               [&](auto& obj) {return obj.type == geometry_type::gui;});
            if(old_gui_object != renderable_objects.end()) {
                renderable_objects.erase(old_gui_object);
                renderable_objects.insert(old_gui_object, gui);
                LOG(DEBUG) << "Replaced the GUI geometry";
            } else {
                renderable_objects.push_back(gui);
                LOG(DEBUG) << "Added GUI geometry";
            }
        }
    }
}
