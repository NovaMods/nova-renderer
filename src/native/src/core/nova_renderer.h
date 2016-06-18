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
     * \brief Checks if Minecraft has recently sent a rendering command
     *
     * The original plan for this mod was to use Vulkan as the rendering API and run the renderer in a completely
     * separate thread from Minecraft itself. This posed a number of excruciating problems, so now I'm using OpenGL from
     * the same thread as Minecraft. This method is, at present, completely superfluous. However, I'd like to stick to
     * my original model of running the renderer in a completely separate thread from the rest of the game. I'm not
     * really sure if OpenGL support sthis, so this method and the infrastructure around it will stay for the time being
     *
     * \return Trus if a render command is available for processing, false otherwise
     */
    bool has_render_available();

    /*!
     * \brief Renders a single frame
     *
     * As stated in the docs for ::has_render_available, the original plan was to have the renderer in a separate
     * thread. This method would be called only if a render command was available, and the thread would just hang out
     * and do nothing if a rendering command was not available. I'm not sure if this is possible with OpenGL, and until
     * I know for sure I'm keeping traces of the original model in the code
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
    shader_store & get_shader_manager();

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
    std::unique_ptr<iwindow> game_window;
    texture_manager tex_manager;

    shader_store shader_manager;
    uniform_buffer_store ubo_manager;

    gui_renderer gui_renderer_instance;
};

#endif //RENDERER_VULKAN_MOD_H
