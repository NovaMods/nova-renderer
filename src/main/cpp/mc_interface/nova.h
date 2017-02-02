/*!
 * \brief Defines the interface that the Java code wil use to interact with the C++ code
 *
 * \author David
 * \date 25-Apr-16
 */

#ifndef RENDERER_NOVA_H
#define RENDERER_NOVA_H

#include "../utils/export.h"

extern "C" {

#include "mc_objects.h"
/*!
 * \brief Initializes the Nova Renderer
 */
NOVA_API void initialize();

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
NOVA_API void add_texture(mc_atlas_texture & texture, int texture_type);


/*!
 * \brief Adds the given location to the list of texture locations
 *
 * The textures are stored in atlases, but I still need to know the UV range for a given block's texture so I can assign
 * the block the proper UV coordinates
 *
 * \param location The location of the texture
 */
NOVA_API void add_texture_location(mc_texture_atlas_location location);

/*!
 * \brief Queries OpenGL and returns the maximum texture size that OpenGL allows
 */
NOVA_API int get_max_texture_size();

/*!
 * \brief Clears out all the textures held by the texture manager, readying the texture manager to receive the textures
 * from a new resource pack
 */
NOVA_API void reset_texture_manager();

/*!
 * \brief Updates the Nova Renderer and renders the current frame
 */
NOVA_API void execute_frame();

/*!
 * \brief Checks if Minecraft should close
 *
 * Since the Nova Renderer creates its own window, it needs to handle window events itself. However, Minecraft still
 * needs to be made aware of any window events. This function, and a couple others, poll the native code for any new
 * window events. This one triggers if the user decides to close Nova's renderer
 *
 * \return Trus if the window should close, false otherwise
 */
NOVA_API bool should_close();

/*!
 * \brief Sets the GUI screen that Nova should render
 *
 * \param set_gui_screen The command to set the GUI screen
 */
NOVA_API void send_change_gui_screen_command(mc_set_gui_screen_command * set_gui_screen);

NOVA_API void send_gui_buffer_command(mc_gui_send_buffer_command * command);

/*!
* \brief Gets the current window size
*/
NOVA_API struct window_size get_window_size();

/*!
* \brief Removes all gui render objects and thereby deletes all the buffers
*/
NOVA_API void clear_gui_buffers();

/**
 * Settings updates
 */

/*!
 * \brief Sets a string setting to a specified value
 *
 * This is super useful when, say, changing the loaded shaderpack
 *
 * \param setting_name The name of the setting to update
 * \param setting_value The desired value of the setting
 */
NOVA_API void set_string_setting(const char * setting_name, const char * setting_value);

NOVA_API struct mouse_button_event  get_next_mouse_button_event();

NOVA_API struct mouse_position_event  get_next_mouse_position_event();

NOVA_API struct key_press_event  get_next_key_press_event();

NOVA_API struct key_char_event  get_next_key_char_event();

};  // End extern C
    // I don't like doing this, but I just saw this closing curly brace and freaked out a little bit.
    // Random closing braces are not okay.

#endif //RENDERER_NOVA_H
