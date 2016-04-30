//
// Created by David on 25-Dec-15.
//

#ifndef RENDERER_VULKAN_MOD_H
#define RENDERER_VULKAN_MOD_H

#include "../interfaces/iwindow.h"
#include "../gl/core/opengl_wrapper.h"
#include "nova.h"
#include "../mc/mc_objects.h"
#include "texture_manager.h"

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
    static nova_renderer * instance;

    nova_renderer();
    ~nova_renderer();

    bool has_render_available();

    void render_frame();

    bool should_continue();

    texture_manager * get_texture_manager();

    static void init_instance();

private:
    opengl_wrapper wrapper;
    iwindow* m_game_window;
    texture_manager tex_manager;
    int num_textures;
};

#endif //RENDERER_VULKAN_MOD_H
