/*!
 * \brief Holds all the objects that we need to get from MC
 *
 * \author David
 * \date 27-Apr-16.
 */

#ifndef RENDERER_MC_OBJECTS_H
#define RENDERER_MC_OBJECTS_H

/*!
 * \brief Holds the information that comes from MC textures
 */
struct mc_atlas_texture {
    int width;
    int height;
    int num_components;
    unsigned char * texture_data;
};

struct mc_texture_atlas_location {
    const char * name;
    float min_u;
    float max_u;
    float min_v;
    float max_v;
};

struct mc_block {
    bool is_on_fire;
    int block_id;
};

struct mc_chunk {
    long chunk_id;
    mc_block * blocks;
};

struct mc_render_world_params {
    double camera_x;
    double camera_y;
    double camera_z;

    int render_distance;

    bool has_blinkness;

    double fog_color_red;
    double fog_color_green;
    double fog_color_blue;

    bool view_bobbing;
    int should_render_clouds;

    mc_chunk * chunks;
    int num_chunks;
};

struct mc_render_command {
    bool render_menu;

    long previous_frame_time;

    float mouse_x;
    float mouse_y;

    bool anaglyph;

    int display_width;
    int display_height;

    mc_render_world_params render_world_params;
};

#endif //RENDERER_MC_OBJECTS_H
