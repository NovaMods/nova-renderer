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
/*!
 * \brief Holds the information that comes from MC textures
 */
struct mc_atlas_texture {
    int width;
    int height;
    int num_components;
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

/*!
 * \brief Represents a block in Minecraft, along with any attributes it might have
 */
struct mc_block {
    bool is_on_fire;
    int block_id;
};

/*!
 * \brief Represents a chunk in Minecraft. It's really just a large array of blocks and an ID so I don't have to rebuild
 * the geometry every frame
 */
struct mc_chunk {
    long chunk_id;  //!< Unique identifier for the chunk
    bool is_dirty;  //!< Has the chunk changed since it was last sent to Nova?
    mc_block blocks[16 * 16 * 16];  //!< All the blocks in the chunk
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

struct mc_gui_send_buffer_command {
    const char *texture_name;  //!< The resource name of the texture.
	int index_buffer_size;
	int vertex_buffer_size;
	int* index_buffer;
    float* vertex_buffer;
	const char* atlas_name;
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
    mc_chunk new_chunk;

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
