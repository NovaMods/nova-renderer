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
    long chunk_id;
    bool is_dirty;
    mc_block * blocks;
};

/*!
 * \brief All the data needed to render the world
 */
struct mc_render_world_params {
    double camera_x;
    double camera_y;
    double camera_z;

    int render_distance;        //!< Should be a setting

    bool has_blinkness;         //!< I forget what this does

    double fog_color_red;       //!< Should be a setting
    double fog_color_green;     //!< Should be a setting
    double fog_color_blue;      //!< Should be a setting

    bool view_bobbing;          //!< Should be a setting
    int should_render_clouds;   //!< Should be a setting

    mc_chunk * chunks;
    int num_chunks;
};

/*!
 * \brief All the data needed to render the GU, along with a couple flags
 */
struct mc_render_gui_params {
    bool render_menu;
    mc_gui_screen cur_screen;
};

/*!
 * \brief All the data needed to render a single frame
 */
struct mc_render_command {
    long previous_frame_time;

    float mouse_x;
    float mouse_y;

    bool anaglyph;      //!< Should be a setting

    int display_width;
    int display_height;

    mc_render_world_params render_world_params;
    mc_render_gui_params render_gui_params;
};

#endif //RENDERER_MC_OBJECTS_H
