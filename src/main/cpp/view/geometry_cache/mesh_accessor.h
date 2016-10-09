/*!
 * \brief
 *
 * \author ddubois 
 * \date 27-Sep-16.
 */

#ifndef RENDERER_GEOMETRY_CACHE_H
#define RENDERER_GEOMETRY_CACHE_H

#include <unordered_map>
#include "view/geometry_cache/gui/gui_geometry_builder.h"
#include "render_object.h"
#include "view/render_pass.h"
#include "geometry_builder.h"

namespace nova {
    namespace view {
        /*!
         * \brief Provides access to the meshes that Nova will want to deal with
         *
         * The primary way it does this is by allowing the user to specify
         */
        class mesh_accessor {
        public:
            std::vector<render_object> get_meshes_for_filter(std::function<bool(render_object)>& filter);

            void add_render_object(const render_object new_obj);

            /*!
             * \brief Provides the geometry builder so that the model can provide things to have geometry built from
             *
             * \return The geometry builder
             */
            geometry_builder& get_geometry_builder();
        private:
            std::mutex render_objects_lock;
            std::vector<render_object> renderable_objects;
            geometry_builder geom_builder;
        };
    }
}

#endif //RENDERER_GEOMETRY_CACHE_H
