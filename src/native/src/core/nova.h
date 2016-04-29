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

#include "nova_renderer.h"
#include "../mc/mc_objects.h"

extern "C" {

/*!
 * \brief Initializes the Nova Renderer
 *
 * How does this initialize the Nova Renderer? Well, the Nova Renderer is a singleton. Why? So I don't have to pass it
 * as a parameter to every single function that this library provides.
 */
void init_nova();

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
void add_texture(mc_texture texture);

};  // End extern C
    // I don't like doing this, but I just say this closing curly brace and freaked out a little bit.
    // Random closing braces are not okay.

#endif //RENDERER_NOVA_H
