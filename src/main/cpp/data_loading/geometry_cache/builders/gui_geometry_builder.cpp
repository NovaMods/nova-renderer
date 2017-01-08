/*!
 * \author David
 * \date 13-May-16.
 */

#include <algorithm>
#define ELPP_THREAD_SAFE
#include <easylogging++.h>
#include <glad/glad.h>
#include "gui_geometry_builder.h"


namespace nova {
    mesh_definition build_gui_geometry(mc_gui_screen &cur_screen) {
        // The UV coordinates for a pressed and an unpressed GUI button
        static auto basic_unpressed_uvs = std::vector<float>{
                0.0f, 0.3359375f,
                0.78125f, 0.3359375f,
                0.0f, 0.4156963f,
                0.78125f, 0.4156963f
        };

        static auto basic_pressed_uvs = std::vector<float>{
                0.0f, 0.2578112f,
                0.78125, 0.2578112f,
                0.0f, 0.3359375f,
                0.78125f, 0.3359375f
        };

        // We need to make a vertex buffer with the positions and texture coordinates of all the gui elements
        std::vector<float> vertex_buffer;
        vertex_buffer.reserve((unsigned long long int) (cur_screen.num_buttons * 4 *
                                                        5));    // cur_screen.num_buttons buttons * 4 vertices per button * 5 elements per vertex

        std::vector<unsigned> indices;
        indices.reserve((unsigned long long int) (cur_screen.num_buttons * 6)); // six entries per button
        unsigned start_pos = 0;

        for(int i = 0; i < cur_screen.num_buttons; i++) {
            mc_gui_button &button = cur_screen.buttons[i];

            // TODO: More switches to figure out exactly which UVs we should use
            std::vector<float> &uv_buffer = basic_unpressed_uvs;
            if(button.is_pressed) {
                uv_buffer = basic_pressed_uvs;
            }

            // Generate the vertexes from the button's position
            add_vertices_from_button(vertex_buffer, button, uv_buffer);

            add_indices_with_offset(indices, start_pos);

            // Add the number of new vertices to the offset for indices, so that indices point to the right
            // vertices
            start_pos += 4;
        }

        mesh_definition cur_screen_buffer;
        cur_screen_buffer.vertex_data = vertex_buffer;
        cur_screen_buffer.indices = indices;
        cur_screen_buffer.vertex_format = format::POS_UV;

        LOG(INFO) << "Build GUI Geometry";

        return cur_screen_buffer;
    }


    void add_indices_with_offset(std::vector<unsigned> &indices, unsigned start_pos) {
        static auto index_buffer = std::vector<unsigned short>{
                0, 1, 2,
                2, 1, 3
        };

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


