/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_GUI_RENDERER_H
#define RENDERER_GUI_RENDERER_H

#include <memory>
#include <atomic>

#include <mutex>
#include <data_loading/geometry_cache/mesh_definition.h>

#include "view/objects/gl_mesh.h"
#include "mc_interface/mc_gui_objects.h"

namespace nova {
    namespace model {
        /*!
         * \brief Constructs the geometry needed to render the current GUI screen
         *
         * Note that the GUI screen does not include things like the spinning background on the main menu screen, because
         * that's going to be rendered as if it was a scene
         */
        NOVA_API mesh_definition build_gui_geometry(mc_gui_screen &cur_screen);

        NOVA_API void add_indices_with_offset(std::vector<unsigned> &indices, unsigned start_pos);

        /*!
         * \brief Adds all the vertices from the given button to the given vertex buffer. uvs holds the uv
         * coordinates for this button
         *
         * \param vertex_buffer The vertex buffer to add vertices to
         * \param button The button to get vertices from
         * \param uvs The uv coordinates to use for this button
         */
        NOVA_API void add_vertices_from_button(std::vector<float> &vertex_buffer, const mc_gui_button &button,
                                      const std::vector<float> &uvs);

        /*!
         * \brief Adds the vertex with the given parameters to the given vertex buffer
         *
         * Note that the z position of the vertices is always set to 0. This is maybe what I want.
         *
         * \param vertex_buffer The thing to add vertices to
         * \param x The x position of the vertex
         * \param y The y position of the vertex
         * \param u The u texture coordiante of the vertex
         * \param v The v texture coordinate of the vertex
         */
        NOVA_API void add_vertex(std::vector<float> &vertex_buffer, int x, int y, float u, float v);
    }
}

#endif //RENDERER_GUI_RENDERER_H
