//
// Created by David on 25-Apr-16.
//

#ifndef RENDERER_NOVA_H
#define RENDERER_NOVA_H

#if defined DLL_EXPORT
#define NOVA_FUNC __declspec(dllexport)
#else
#define NOVA_FUNC __declspec(dllimport)
#endif

#include "../mc/mc_objects.h"

extern "C" {

/*!
 * \brief Initializes the Nova Renderer
 *
 * How does this initialize the Nova Renderer? Well, the Nova Renderer is a singleton. Why? So I don't have to pass it
 * as a parameter to every single function that this library provides.
 */
NOVA_FUNC void init_nova();

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
NOVA_FUNC void add_texture(mc_atlas_texture & texture, int atlas_type, int texture_type);

/*!
 * \brief Adds the given location to the list of texture locations
 *
 * The textures are stored in atlases, but I still need to know the UV range for a given block's texture so I can assign
 * the block the proper UV coordinates
 *
 * \param location The location of the texture
 */
NOVA_FUNC void add_texture_location(mc_texture_atlas_location location);

/*!
 * \brief Queries OpenGL and returns the maximum texture size that OpenGL allows
 */
NOVA_FUNC int get_max_texture_size();

NOVA_FUNC void reset_texture_manager();

};  // End extern C
    // I don't like doing this, but I just say this closing curly brace and freaked out a little bit.
    // Random closing braces are not okay.

#endif //RENDERER_NOVA_H
