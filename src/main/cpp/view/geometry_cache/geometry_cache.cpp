/*!
 * \brief
 *
 * \author ddubois 
 * \date 27-Sep-16.
 */

#include "geometry_cache.h"

namespace nova {
    namespace view {
        model::render_object& geometry_cache::get_chunk_object(unsigned int chunk_id) const {
            auto chunk_itr = chunks.find(chunk_id);

            if(chunk_itr == chunks.end()) {
                // Build the geometry for this chunk and put it in the map
            }

            return chunks[chunk_id];
        }

        model::gl_vertex_buffer& geometry_cache::get_gui_geometry() {
            return gui_geometry.get_vertex_buffer();
        }
    }
}
