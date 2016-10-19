/*!
 * \author David
 * \date 13-May-16.
 */

#include <algorithm>
#include <easylogging++.h>
#include "gui_geometry_builder.h"


namespace nova {
    namespace model {
        std::vector<float> basic_unpressed_uvs = {
                0.0f, 0.3359375f,
                0.78125f, 0.3359375f,
                0.0f, 0.4156963f,
                0.78125f, 0.4156963f
        };

        std::vector<float> basic_pressed_uvs = {
                0.0f, 0.2578112f,
                0.78125, 0.2578112f,
                0.0f, 0.3359375f,
                0.78125f, 0.3359375f
        };

        std::vector<unsigned short> index_buffer = {
                0, 1, 2,
                2, 1, 3
        };

        void add_indices_for_button(std::vector<unsigned short> &indices, unsigned short start_pos);

        /*!
         * \brief Adds all the vertices from the given button to the given vertex buffer. uvs holds the uv
         * coordinates for this button
         *
         * \param vertex_buffer The vertex buffer to add vertices to
         * \param button The button to get vertices from
         * \param uvs The uv coordinates to use for this button
         */
        void add_vertices_from_button(std::vector<float> &vertex_buffer, const mc_gui_button &button,
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
        void add_vertex(std::vector<float> &vertex_buffer, int x, int y, float u, float v);

        model::gl_mesh build_gui_geometry(mc_gui_screen& cur_screen) {
            // We need to make a vertex buffer with the positions and texture coordinates of all the gui elements
            std::vector<float> vertex_buffer(
                    MAX_NUM_BUTTONS * 4 * 5
            );    // MAX_NUM_BUTTONS buttons * 4 vertices per button * 5 elements per vertex
            std::vector<unsigned short> indices;
            unsigned short start_pos = 0;

            for(int i = 0; i < cur_screen.num_buttons; i++) {
                mc_gui_button &button = cur_screen.buttons[i];

                // TODO: More switches to figure out exactly which UVs we should use
                std::vector<float> &uv_buffer = basic_unpressed_uvs;
                if(button.is_pressed) {
                    uv_buffer = basic_pressed_uvs;
                }

                // Generate the vertexes from the button's position
                add_vertices_from_button(vertex_buffer, button, uv_buffer);

                add_indices_for_button(indices, start_pos);

                // Add the number of new vertices to the offset for indices, so that indices point to the right
                // vertices
                start_pos += 4;
            }

            model::gl_mesh cur_screen_buffer;

            cur_screen_buffer.set_data(
                    vertex_buffer, ivertex_buffer::format::POS_UV, ivertex_buffer::usage::static_draw
            );

            cur_screen_buffer.set_index_array(indices, ivertex_buffer::usage::static_draw);

            return cur_screen_buffer;
        }


        void add_indices_for_button(std::vector<unsigned short> &indices, unsigned short start_pos) {
            for(unsigned short &index : index_buffer) {
                indices.push_back(index + start_pos);
            }
        }

        void add_vertices_from_button(std::vector<GLfloat> &vertex_buffer, const mc_gui_button &button,
                                      const std::vector<GLfloat> &uvs) {
            add_vertex(
                    vertex_buffer,
                    button.x_position, button.y_position,
                    uvs[0], uvs[1]
            );
            add_vertex(
                    vertex_buffer,
                    button.x_position + button.width, button.y_position,
                    uvs[2], uvs[3]
            );
            add_vertex(
                    vertex_buffer,
                    button.x_position, button.y_position + button.height,
                    uvs[4], uvs[5]
            );
            add_vertex(
                    vertex_buffer,
                    button.x_position + button.width, button.y_position + button.height,
                    uvs[6], uvs[7]
            );
        }

        void add_vertex(std::vector<float> &vertex_buffer, int x, int y, float u, float v) {
            vertex_buffer.push_back(static_cast<float>(x));
            vertex_buffer.push_back(static_cast<float>(y));
            vertex_buffer.push_back(0.0f);

            vertex_buffer.push_back(u);
            vertex_buffer.push_back(v);
        }
    }
}


