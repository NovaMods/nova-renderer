//
// Created by David on 25-Dec-15.
//

#ifndef RENDERER_VULKAN_MOD_H
#define RENDERER_VULKAN_MOD_H

#include <memory>

#include "../interfaces/iwindow.h"
#include "nova.h"
#include "../mc/mc_objects.h"
#include "texture_manager.h"
#include "gui/gui_renderer.h"
#include "types.h"

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
    static std::unique_ptr<nova_renderer> instance;

    nova_renderer();
    ~nova_renderer();

    bool has_render_available();

    void render_frame();

    bool should_end();

    texture_manager & get_texture_manager();

    static void init_instance();

    shader_store & get_shader_manager();

    uniform_buffer_store & get_ubo_manager();

    gui_renderer & get_gui_renderer();

private:
    std::unique_ptr<iwindow> game_window;
    texture_manager tex_manager;

    shader_store shader_manager;
    uniform_buffer_store ubo_manager;

    gui_renderer gui_renderer_instance;
    int num_textures;
};

#endif //RENDERER_VULKAN_MOD_H
