/*!
 * \brief Holds all the objects that we need to get from MC
 *
 * \author David
 * \date 27-Apr-16.
 */

#ifndef RENDERER_MC_OBJECTS_H
#define RENDERER_MC_OBJECTS_H

#include "mc_gui_objects.h"

/*!
 * \brief Holds the information that comes from MC textures
 */
struct mc_atlas_texture {
    int width;
    int height;
    int num_components;
    unsigned char * texture_data;
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

#endif //RENDERER_MC_OBJECTS_H
