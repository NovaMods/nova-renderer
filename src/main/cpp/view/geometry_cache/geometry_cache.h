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
#include "model/render_object.h"

namespace nova {
    namespace view {
        class geometry_cache {
        public:
            model::render_object& get_chunk_object(unsigned int chunk_id) const;

            model::gl_vertex_buffer& get_gui_geometry();

        private:
            std::unordered_map<unsigned int, model::render_object> chunks;

            gui_geometry_builder gui_geometry;
        };
    }
}

#endif //RENDERER_GEOMETRY_CACHE_H
