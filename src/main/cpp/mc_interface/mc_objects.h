/*!
 * \brief Holds all the objects that we need to get from MC
 *
 * \author David
 * \date 27-Apr-16.
 */

#ifndef RENDERER_MC_OBJECTS_H
#define RENDERER_MC_OBJECTS_H

#include "mc_gui_objects.h"
#include <cstdint>

const int CHUNK_WIDTH = 16;
const int CHUNK_HEIGHT = 256;
const int CHUNK_DEPTH = 16;

/*!
 * \brief Holds the information that comes from MC textures
 */
struct mc_atlas_texture {
    uint32_t width;
    uint32_t height;
    uint32_t num_components;
    unsigned char * texture_data;
	const char * name;
};

/*!
 * \brief Holds the information of where in a texture atlas a given texture is
 */
struct mc_texture_atlas_location {
    const char * name;  //!< The resource name of the texture. Should be identical to the string used to acwuire the texture
    float min_u;
    float max_u;
    float min_v;
    float max_v;
};

struct mc_block_definition {
	const char * name;
	int light_opacity;
    int light_value;
	int is_opaque;
	int block_light;
	int is_cube;

	bool is_emissive() const;
	bool is_transparent() const;
};

/*!
 * \brief Represents a block in Minecraft, along with any attributes it might have
 */
struct mc_block {
	int id;
    bool is_on_fire;
	float ao;
    char * state;
};

/*!
 * \brief A vertex from a chunk
 *
 * Format:
 * XYZ 	3 32-bit floats
 * RGBA 4 bytes
 * UV0 	2 floats
 * UV1 	2 16-bit floats
 *
 * 12 bytes for position (XYZ)
 * 4 bytes for color (RGBA)
 * 8 bytes for main UV (UV)
 * 4 bytes for secondary UV (UV)
 *
 * 26 bytes total
 */
struct mc_block_vertex {
	float x;
	float y;
	float z;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
	float uv0_u;
	float uv0_v;
	uint16_t uv1_u;
    uint16_t uv1_v;
};

/*!
 * \brief Represents a chunk in Minecraft. It's really just a large array of blocks and an ID
 */
struct mc_chunk_render_object {
	int format;
	float x;
	float y;
	float z;
	int id;
	mc_block_vertex* vertex_data;
	int* indices;
	int vertex_buffer_size;
	int index_buffer_size;

};

/*!
 * \brief Represents a single quad in Minecraft
 */
struct mc_baked_quad {
	int num_vertices;
	int tint_index;
	const char * texture_name;
    int vertex_data[28];
};

/*!
 * \brief Represents a simple model (i.e. a model with only one variant (I think))
 */
struct mc_baked_model {
    const char * block_state;
	int num_quads;
    mc_baked_quad* quads;
};

/*!
 * \brief Describes a single entity
 */
struct mc_entity {
    int id;
};

/*!
 * \brief All the data needed to render the world
 */
struct mc_render_world_params {
    double camera_x;
    double camera_y;
    double camera_z;
};

/*!
 * \brief All the data needed to render the GU, along with a couple flags
 */
struct mc_render_gui_params {
    mc_gui_screen cur_screen;
};

struct mc_gui_geometry {
    const char *texture_name;  //!< The resource name of the texture.
	int index_buffer_size;
	int vertex_buffer_size;
	int* index_buffer;
    float* vertex_buffer;
	const char * atlas_name;
};

/*!
 * \brief Holds all the settings that are exposed from the Minecraft options menu
 */
struct mc_settings {
    bool render_menu;           //!< Controlled by F1

    bool anaglyph;

    double fog_color_red;
    double fog_color_green;
    double fog_color_blue;

    int display_width;
    int display_height;

    bool view_bobbing;
    int should_render_clouds;

    int render_distance;

    bool has_blindness;         //!< Some potion effect I think
};

/*!
 * \brief Tells Nova to add this chunk to its list of potential chunks to render
 */
struct mc_add_chunk_command {
    mc_chunk_render_object new_chunk;

    float chunk_x;
    float chunk_y;
    float chunk_z;
};

/*!
 * \brief Tells Nova to change to the give GUI screen
 */
struct mc_set_gui_screen_command {
    mc_gui_screen screen;
};

struct mouse_button_event {
	int button;
	int action;
	int mods;
	int filled;
};


struct mouse_position_event {
	int xpos;
	int ypos;
	int filled;
};

struct mouse_scroll_event {
    double xoffset;
    double yoffset;
    int filled;
};

struct key_press_event {
	int key;
	int scancode;
	int action;
	int mods;
	int filled;

};

struct key_char_event {
	std::uint64_t unicode_char;
	int filled;

};

struct window_size {
    int height;
    int width;
};
#endif //RENDERER_MC_OBJECTS_H
