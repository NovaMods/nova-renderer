//
// Created by David on 25-Dec-15.
//

#ifndef RENDERER_VULKAN_MOD_H
#define RENDERER_VULKAN_MOD_H

#include <memory>
#include <pthread.h>    // I'd love to use <thread>, but apparently my compiler doesn't support it. Just a few more days until Linux...

#include "interfaces/iwindow.h"
#include "mc/mc_objects.h"

#include "nova.h"
#include "texture_manager.h"
#include "gui/gui_renderer.h"
#include "config/config.h"
#include "shaderpack_loading/shaderpack.h"
#include "uniform_buffer_store.h"
#include "../gl/windowing/glfw_gl_window.h"

/*!
 * \brief Initializes everything this mod needs, creating its own window
 *
 * Idea: Replace the Display class too, maybe?
 *
 * This class is kinda a facade and kinda a God class that holds all the everything that the mod needs. I'd like it to
 * be more of a facade but idk. Facades are hard.
 *
 * This class's instance runs completely in a separate thread. Whatever you want to use it for, it runs in a separate
 * thread. Calling froman application with a million threads already? Too bad, separate thread.
 *
 * I'm not worried about data races. Data moves into this thread, then gets rendered. The only data racey thing are the
 * flags that specify rendering commands are available. However, I'm using atomics for those, so I don't expect too many
 * problems. If I notice the renderer missing render commands, I'll re-evaluate the data integrity scheme
 */
class nova_renderer {
public:
    /*!
     * \brief A singleton for the nova_renderer instance
     *
     * I want only one nova_renderer active at a time, and the C code needs a good way to reference the nova_renderer
     * instance. A singleton fulfils both those requirements.
     */
    static std::unique_ptr<nova_renderer> instance;

    /*!
     * \brief Initializes the nova_renderer instance
     *
     * See \ref ::nova_renderer for an overview of what all that entails
     */
    static void init_instance();

    /*!
     * \brief Initiazes the nova_renderer
     *
     * Initializing the nova_renderer is a lot of work. I create an OpenGL context, create a GLFW window, initialize
     * the texture manager, shader manager, UBO manager, etc. and set up initial OpenGL state.
     */
    nova_renderer();

    /*!
     * \brief Destructor
     */
    ~nova_renderer();

    /*!
     * \brief Renders a single frame
     *
     * This method runs in a separate thread from the rest of the methods in this class. This is because the other
     * methods are called by Minecraft, from the Minecraft thread, while this method is run in a separate thread. This
     * is done to keep the OpenGL contexts separate. Hopefully it'll work
     */
    void render_frame();

    /*!
     * \brief determines whether or not the Nova Renderer, and by extension Minecraft, should shut down. Called directly
     * by the C interface
     *
     * \return True if we should shutdown, false otherwise
     */
    bool should_end();

    /*!
     * \brief Returns the texture manager
     */
    texture_manager & get_texture_manager();

    /*!
     * \brief Returns the shader manager
     */
    shaderpack & get_shaderpack();

    /*!
     * \brief Returns the uniform buffer object manager
     */
    uniform_buffer_store & get_ubo_manager();

    /*!
     * \brief Returns the gui renderer
     *
     * I use this method now for testing. I expect that, upon release, the gui_renderer will only be called from within
     * this class and thus will not need to be gotten at all, but we'll see.
     */
    gui_renderer & get_gui_renderer();

private:
    static pthread_t render_thread;

    glfw_gl_window game_window;
    texture_manager tex_manager;

    shaderpack shaders;
    uniform_buffer_store ubo_manager;

    gui_renderer gui_renderer_instance;

    config nova_config;

    void enable_debug();
};

#endif //RENDERER_VULKAN_MOD_H
