//
// Created by David on 25-Dec-15.
//

#ifndef RENDERER_VULKAN_MOD_H
#define RENDERER_VULKAN_MOD_H

#include "../interfaces/iwindow.h"
#include "../gl/core/opengl_wrapper.h"
#include "nova.h"
#include "../mc/mc_objects.h"

/*!
 * \brief Initializes everything this mod needs, creating its own window
 *
 * Idea: Replace the Display class too, maybe?
 *
 * This class is kinda a facade and kinda a God class that holds all the everything that the mod needs. I'd like it to
 * be more of a facade but idk. Facades are hard.
 */
class nova_renderer {
public:
    nova_renderer();
    ~nova_renderer();

    bool has_render_available();

    void render_frame();

    bool should_continue();

    void add_texture(mc_texture & new_texture);

    static void init_instance();
    static nova_renderer * get_instance();

private:
    static nova_renderer * instance;

    opengl_wrapper wrapper;
    iwindow* m_game_window;
    int num_textures;
};

#endif //RENDERER_VULKAN_MOD_H
