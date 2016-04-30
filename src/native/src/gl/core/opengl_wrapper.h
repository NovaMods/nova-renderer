//
// Created by David on 01-Apr-16.
//

#ifndef RENDERER_OPENGL_WRAPPER_H
#define RENDERER_OPENGL_WRAPPER_H


#include "../objects/texture2D.h"

/*!
 * \brief Provides a higher-level API into OpenGL, providing convenient methods for creating the OpenGL instance,
 * managing the creating of devices, and whatever else
 */
class opengl_wrapper {
public:
    /*!
     * \brief Initializes OpenGL and whatnot
     */
    opengl_wrapper();

    /*!
     * \brief Allocates a new texture on the GPU
     *
     * \return The new texture
     */
    texture2D make_texture_2D();

private:
};


#endif //RENDERER_OPENGL_WRAPPER_H
