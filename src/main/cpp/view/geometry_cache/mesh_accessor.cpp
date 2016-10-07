/*!
 * \brief
 *
 * \author ddubois 
 * \date 27-Sep-16.
 */

#include <d3d11.h>
#include <vector>
#include "mesh_accessor.h"

namespace nova {
    namespace view {
        std::vector<render_object> mesh_accessor::get_meshes_for_filter(std::function<bool(render_object)> &filter) {
            std::vector<render_object> meshes(renderable_objects.size());

            render_objects_lock.lock();
            for(auto& renderable_object : renderable_objects) {
                if(filter(renderable_object)) {
                    meshes.push_back(renderable_object);
                }
            }
            render_objects_lock.unlock();

            return meshes;
        }

        void mesh_accessor::add_render_object(const render_object new_obj) {
            render_objects_lock.lock();

            renderable_objects.push_back(new_obj);

            render_objects_lock.unlock();
        }
    }
}
