/*!
 * \brief
 *
 * \author ddubois 
 * \date 27-Sep-16.
 */

#ifndef RENDERER_GEOMETRY_CACHE_H
#define RENDERER_GEOMETRY_CACHE_H

#include <vector>
#include <mutex>
#include <functional>
#include <unordered_map>
#include <data_loading/geometry_cache/builders/mesh_builder.h>
#include "mc_interface/mc_objects.h"
#include "render_object.h"
#include "view/render_pass.h"

namespace nova {
    namespace view {
        /*!
         * \brief Provides access to the meshes that Nova will want to deal with
         *
         * The primary way it does this is by allowing the user to specify
         */
        class mesh_accessor {
        public:
            /*!
             * \brief Initializes this mesh_accessor, telling it to get new meshes from the given mesh_builder
             *
             * \param builder_ref The mesh_builder to get new meshes from
             */
            mesh_accessor(model::mesh_builder& builder_ref);

            /*!
             * \brief Checks the mesh_builder for new mesh definitions. If we have any, creates an OpenGL mesh for them
             */
            void update();

            std::vector<render_object*> get_meshes_for_filter(std::function<bool(const render_object&)>& filter);
        private:
            std::vector<render_object> renderable_objects;

            model::mesh_builder& mesh_builder;

            void update_gui_mesh();
        };
    }
}

#endif //RENDERER_GEOMETRY_CACHE_H
