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

/**
 * Textures
 */

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
NOVA_API void add_texture(mc_atlas_texture & texture);

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
 * \brief Adds a chunk to Minecraft, or updates an existing chunk
 *
 * Chunks are identified by their chunk ID. Nova maintains a mapping from chunk ID to render_objects for that chunk.
 * This lets Nova clean out the geometry for an old chunk to make room for a new chunk
 *
 * \param chunk The chunk to add to Nova
 */
NOVA_API void add_chunk(mc_chunk & chunk);

/*!
 * \brief Registers a simple model with Nova
 *
 * \param model_name The string name of the model
 * \param model The geometry for the model
 */
NOVA_API void register_simple_model(const char * model_name, mc_simple_model * model);

/*!
 * \brief Removes a model from Nova
 *
 * \param model_name The name of the model to remove
 */
NOVA_API void deregister_model(const char * model_name);

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
 * \return True if the window should close, false otherwise
 */
NOVA_API bool should_close();

/*!
 * \brief Checks if the window has focus
 *
 * \return True if the GLFW window is active, false otherwise
 */
NOVA_API bool display_is_active();

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
 * \brief Sets the fullscreen mode to 
 * \param fullscreen true if 1, false otherwise
 */
NOVA_API void set_fullscreen(int fullscreen);
/*!
 * \brief Sets a string setting to a specified value
 *
 * This is super useful when, say, changing the loaded shaderpack
 *
 * \param setting_name The name of the setting to update
 * \param setting_value The desired value of the setting
 */
NOVA_API void set_string_setting(const char * setting_name, const char * setting_value);
/*!
* \brief Sets a float setting to a specified value
*
* This is super useful when, say, changing the loaded shaderpack
*
* \param setting_name The name of the setting to update
* \param setting_value The desired value of the setting
*/
NOVA_API void set_float_setting(const char * setting_name, float setting_value);

/*!
 * \brief Sets the player camera's location and rotation to the given values
 *
 * \param x The X-coordinate of the camera's position
 * \param y The y-coordinate of the camera's position
 * \param z The X-coordinate of the camera's position
 * \param yaw The camera's yaw
 * \param pitch The camera's pitch
 */
//NOVA_API void set_player_camera_transform(float x, float y, float z, float yaw, float pitch);


/**
 * Pass mouse and key events to Minecraft
 */

NOVA_API struct mouse_button_event  get_next_mouse_button_event();

NOVA_API struct mouse_position_event  get_next_mouse_position_event();

NOVA_API struct mouse_scroll_event  get_next_mouse_scroll_event();

NOVA_API struct key_press_event  get_next_key_press_event();

NOVA_API struct key_char_event  get_next_key_char_event();

};  // End extern C
    // I don't like doing this, but I just saw this closing curly brace and freaked out a little bit.
    // Random closing braces are not okay.

#endif //RENDERER_NOVA_H
