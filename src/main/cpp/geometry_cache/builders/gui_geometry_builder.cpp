/*!
 * \author David
 * \date 13-May-16.
 */

#include <algorithm>
#define ELPP_THREAD_SAFE
#include <easylogging++.h>
#include <glad/glad.h>
#include "gui_geometry_builder.h"
#include "../../mc_interface/mc_objects.h"
#include "../../render/nova_renderer.h"


namespace nova {
    mesh_definition build_gui_geometry(mc_gui_screen &cur_screen) {
        // The UV coordinates for a pressed and an unpressed GUI button
        static auto basic_hovered_uvs = std::vector<float>{
                0.0f, 0.3359375f,
                0.78125f, 0.3359375f,
                0.0f, 0.4156963f,
                0.78125f, 0.4156963f
        };

		static auto basic_unpressed_uvs = std::vector<float>{
				0.0f, 0.2578112f,
				0.78125, 0.2578112f,
				0.0f, 0.3359375f,
				0.78125f, 0.3359375f
		};

        static auto basic_pressed_uvs = std::vector<glm::vec2>{
                { 0.0f,         0.1803921f },
                { 0.7803921f,   0.1803921f },
                { 0.0f,         0.2549019f },
                { 0.7803921f,   0.2549019f }
        };

        // We need to make a vertex buffer with the positions and texture coordinates of all the gui elements
        std::vector<float> vertex_buffer;
        vertex_buffer.reserve((unsigned long long int) (cur_screen.num_buttons * 4 *
                                                        5));    // cur_screen.num_buttons buttons * 4 vertices per button * 5 elements per vertex

		std::vector<unsigned short> indices;
		indices.reserve((unsigned long long int) (cur_screen.num_buttons * 2 * 6)); // six entries per button
		unsigned short start_pos = 0;

        // Scale the UVs in the uv buffer to match what's in the texture atlas
        const texture_manager::texture_location widgets_location = nova_renderer::instance->get_texture_manager().get_texture_location("minecraft:gui/widgets");
        glm::vec2 widgets_size = widgets_location.max - widgets_location.min;

        for(int i = 0; i < cur_screen.num_buttons; i++) {
            mc_gui_button &button = cur_screen.buttons[i];

            auto uv_buffer = basic_unpressed_uvs;
            if(button.is_pressed) {
                uv_buffer = basic_pressed_uvs;
            }

            // It's inefficient to scale the UVs for every button, and to copy the UVs for every button. Better to scale
            // the static vector in this function, but then we'd have to un-scale and re-scale whenever a new
            // resourcepack is loaded... this is kinda gross but it should work and the infrequency of changing GUI
            // screens, coupled with a lot number of buttons per screen, should make this work kinda alright
            for(int cur_uv = 0; cur_uv < uv_buffer.size(); cur_uv++) {
                uv_buffer[cur_uv] = (uv_buffer[cur_uv] * widgets_size) + widgets_location.min;
            }

            // Generate the vertexes from the button's position
            add_vertices_from_button(vertex_buffer, button, uv_buffer);

			add_indices_with_offset(indices, start_pos);
			add_indices_with_offset(indices, start_pos + 4);

			// Add the number of new vertices to the offset for indices, so that indices point to the right
			// vertices
			start_pos += 8;
		}

		mesh_definition cur_screen_buffer;
		cur_screen_buffer.vertex_data = vertex_buffer;
		cur_screen_buffer.indices = indices;
		cur_screen_buffer.vertex_format = format::POS_UV;

		LOG(INFO) << "Build GUI Geometry";

		return cur_screen_buffer;
	}


	void add_indices_with_offset(std::vector<unsigned short> &indices, unsigned short start_pos) {
		static auto index_buffer = std::vector<unsigned short>{
				0, 1, 2,
				2, 1, 3
		};

		for (unsigned short &index : index_buffer) {
			indices.push_back(index + start_pos);
		}
	}

	void add_vertices_from_button(std::vector<GLfloat> &vertex_buffer, const mc_gui_button &button) {
		int i = 1;
		if (button.enabled != 1) {
			i = 0;
		}
		float texY = 46 + i * 20;
		float texX = 0;

		create_rectangle(vertex_buffer, button.x_position, button.y_position, texX, texY, button.width / 2, button.height);
		create_rectangle(vertex_buffer, button.x_position + button.width / 2, button.y_position, 200 - button.width / 2, texY, button.width / 2, button.height);

	}

	void create_rectangle(std::vector<GLfloat> &vertex_buffer, int xpos, int ypos, int texX, int texY, int width, int height) {
		add_vertex(
			vertex_buffer,
			xpos, ypos,
			texX*factor, texY*factor
		);
		add_vertex(
			vertex_buffer,
			xpos + width, ypos,
			(texX + width)*factor, texY*factor
		);
		add_vertex(
			vertex_buffer,
			xpos, ypos + height,
			texX*factor, (texY + height)*factor
		);
		add_vertex(
			vertex_buffer,
			xpos + width, ypos + height,
			(texX + width)*factor, (texY + height)*factor
		);
	}

    void add_vertex(std::vector<float> &vertex_buffer, int x, int y, glm::vec2 uv) {
        vertex_buffer.push_back(static_cast<float>(x));
        vertex_buffer.push_back(static_cast<float>(y));
        vertex_buffer.push_back(0.0f);

        vertex_buffer.push_back(uv.s);
        vertex_buffer.push_back(uv.t);
    }
}


