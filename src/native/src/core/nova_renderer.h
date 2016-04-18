//
// Created by David on 25-Dec-15.
//

#ifndef RENDERER_VULKAN_MOD_H
#define RENDERER_VULKAN_MOD_H

#include "../interfaces/iwindow.h"
#include "../gl/core/opengl_wrapper.h"

/*!
 * \brief Initializes everything this mod needs, creating its own window
 *
 * Idea: Replace the Display class too, maybe?
 */
class nova_renderer {
public:
    nova_renderer();
    ~nova_renderer();

    bool has_render_available();

    void render_frame();

    bool should_continue();

private:
    opengl_wrapper wrapper;
    iwindow* m_game_window;
};

/*!
 * \brief Initializes the Vulkan Mod, starting it up and whatnot
 */
long run_vulkan_mod();


#endif //RENDERER_VULKAN_MOD_H
