/*!
 * \author David
 * \date 13-May-16.
 */

#include <algorithm>
#include <easylogging++.h>
#include <glad/glad.h>
#include "gui_geometry_builder.h"
#include "../../mc_interface/mc_objects.h"
#include "../../render/nova_renderer.h"


namespace nova {
	mesh_definition build_gui_geometry(mc_gui_screen &cur_screen) {

		// The UV coordinates for a enabled standard GUI button
		static texture_manager::texture_location std_button_loc = {
			{ 0.0f ,0.2578112f },
			{ 0.78125f ,0.3359375f }
		};

		// We need to make a vertex buffer with the positions and texture coordinates of all the gui elements
		std::vector<float> vertex_buffer;
		vertex_buffer.reserve((unsigned long long int) (cur_screen.num_buttons * 2 * 4 *
			5));    // cur_screen.num_buttons buttons * 2 rectangles pre button * 4 vertices per button * 5 elements per vertex

		std::vector<unsigned short> indices;
		indices.reserve((unsigned long long int) (cur_screen.num_buttons * 2 * 6)); // six entries per button * 2 rectangles pre button
		unsigned short start_pos = 0;

		// Scale the UVs in the uv buffer to match what's in the texture atlas
		const texture_manager::texture_location widgets_location = nova_renderer::instance->get_texture_manager().get_texture_location("minecraft:gui/widgets");
		glm::vec2 widgets_size = widgets_location.max - widgets_location.min;

		for (int i = 0; i < cur_screen.num_buttons; i++) {
			mc_gui_button &button = cur_screen.buttons[i];

			// It's inefficient to scale the UVs for every button, and to copy the UVs for every button. Better to scale
			// the static vector in this function, but then we'd have to un-scale and re-scale whenever a new
			// resourcepack is loaded... this is kinda gross but it should work and the infrequency of changing GUI
			// screens, coupled with a lot number of buttons per screen, should make this work kinda alright


			texture_manager::texture_location button_loc_in_atlas = {
				(std_button_loc.min * widgets_size) + widgets_location.min ,
				(std_button_loc.max * widgets_size) + widgets_location.min
			};
			if (button.enabled != 1) {
				button_loc_in_atlas.min.y -= 0.078125f * widgets_size.y;
				button_loc_in_atlas.max.y -= 0.078125f * widgets_size.y;
			}
			// Generate the vertexes from the button's position
			create_rectangle(vertex_buffer, button.x_position, button.y_position, button_loc_in_atlas.min.x, button_loc_in_atlas.min.y, button.width / 2, button.height, widgets_size);
			create_rectangle(vertex_buffer, button.x_position + button.width / 2, button.y_position, button_loc_in_atlas.max.x - ((button.width / 2) * factor * widgets_size.x), button_loc_in_atlas.min.y, button.width / 2, button.height, widgets_size);

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

	void add_vertices_from_button(std::vector<GLfloat> &vertex_buffer, const mc_gui_button &button, const std::vector<glm::vec2> uvs) {


	}

	void create_rectangle(std::vector<GLfloat> &vertex_buffer, int xpos, int ypos, float texX, float texY, int width, int height, glm::vec2 atlas_scale_vector) {
		add_vertex(
			vertex_buffer,
			xpos, ypos,
			{ texX, texY }
		);
		add_vertex(
			vertex_buffer,
			xpos + width, ypos,
			{ texX + (width * factor * atlas_scale_vector.x), texY }
		);
		add_vertex(
			vertex_buffer,
			xpos, ypos + height,
			{ texX,texY + (height * factor * atlas_scale_vector.y) }
		);
		add_vertex(
			vertex_buffer,
			xpos + width, ypos + height,
			{ texX + (width * factor * atlas_scale_vector.x), texY + (height * factor * atlas_scale_vector.y) }
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


