/*!
 * \brief Defines the interface that the Java code wil use to interact with the C++ code
 *
 * \author David
 * \date 25-Apr-16
 */

#ifndef RENDERER_NOVA_H
#define RENDERER_NOVA_H

#include "utils/export.h"

extern "C" {

#include "mc/mc_objects.h"

/*!
 * \brief Initializes the Nova Renderer
 *
 * How does this initialize the Nova Renderer? Well, the Nova Renderer is a singleton. Why? So I don't have to pass it
 * as a parameter to every single function that this library provides.
 */
NOVA_EXPORT void init_nova();

/*!
 * \brief Adds a new texture to the Nova Renderer, allowing the native code to use that texture
 *
 * Presumably you're reading this because you want to make the code better. Hopefully you know why I'd want to give a
 * renderer access to a texture.
 *
 * The renderer isn't a parameter to this function because it's a static variable of the \class nova_renderer class.
 *
 * \param texture The texture to add to the renderer
 */
NOVA_EXPORT void add_texture(mc_atlas_texture & texture, int atlas_type, int texture_type);

/*!
 * \brief Adds the given location to the list of texture locations
 *
 * The textures are stored in atlases, but I still need to know the UV range for a given block's texture so I can assign
 * the block the proper UV coordinates
 *
 * \param location The location of the texture
 */
NOVA_EXPORT void add_texture_location(mc_texture_atlas_location location);

/*!
 * \brief Queries OpenGL and returns the maximum texture size that OpenGL allows
 */
NOVA_EXPORT int get_max_texture_size();

/*!
 * \brief Clears out all the textures held by the texture manager, readying the texture manager to receive the textures
 * from a new resource pack
 */
NOVA_EXPORT void reset_texture_manager();

/*!
 * \brief Updates the Nova Renderer, which mostly involves taking data from the queues
 */
NOVA_EXPORT void update_renderer();

/*!
 * \brief Sends the C++ code a command to render a fram
 *
 * \param cmd The mc_render_command containing all the data to be rendered for the current frame
 *
 * See \ref mc_render_command for a description of the render command
 */
NOVA_EXPORT void send_render_command(mc_render_command * command);

/*!
 * \brief Checks if Minecraft should close
 *
 * Since the Nova Renderer creates its own window, it needs to handle window events itself. However, Minecraft still
 * needs to be made aware of any window events. This function, and a couple others, poll the native code for any new
 * window events. This one triggers if the user decides to close Nova's renderer
 *
 * \return Trus if the window should close, false otherwise
 */
NOVA_EXPORT bool should_close();

/*!
 * \brief Sets the GUI screen that Nova should render
 *
 * \param set_gui_screen The command to set the GUI screen
 */
NOVA_EXPORT void send_change_gui_screen_command(mc_set_gui_screen_command * set_gui_screen);

};  // End extern C
    // I don't like doing this, but I just saw this closing curly brace and freaked out a little bit.
    // Random closing braces are not okay.

#endif //RENDERER_NOVA_H
