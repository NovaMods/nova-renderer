//
// Created by David on 26-Apr-16.
//

#ifndef RENDERER_TEXTURE_H
#define RENDERER_TEXTURE_H


#include "../glad/glad.h"
#include "../../interfaces/itexture.h"

class texture2D : public itexture {
public:
    /*!
     * \brief Sets up a texture with the data from an allocator
     *
     * The idea of this is that the thing called when a resource pack is loaded will allocate all the textures ie needs,
     * then pack those allocated textures into texture2D objects so they're easier to deal with. I expect a minimum
     * number of allocations: One for the albedo texture, one for the normal texture, and one for the specular data
     * texture. I might need more, for things like particles of all the random UI textures (Those can be packed into an
     * atlas...)
     *
     * \param width The width of this texture
     * \param height The height of this texture
     * \oaram format The format of this texture
     * \oaram gl_name The OpenGl name of this texture, generated when the texture was allocated
     */
    texture2D(int width, int height, GLint format, GLuint gl_name);

    /*!
     * \brief Binds this texture to the OpenGL context, saving the previously bound texture to an internal variable.
     *
     * For best performance, call this function once, do ABSOLUTELY EVERYTHING you need to do to this texture, then
     * unbind it
     */
    void bind();

    /*!
     * \brief Unbinds this texture from the OpenGL context, binding the previously bound texture
     */
    void unbind();

    /*!
     * \brief Returns the width of this texture
     *
     * \return The width of this texture
     */
    int get_width();

    /*!
     * \brief returns the height of this texture
     *
     * \return The height of this texture
     */
    int get_height();

    /*!
     * \brief Returns the format of this texture, as assigned in the constructor
     *
     * \return The format of this texture
     */
    GLint get_format();
private:
    int width;
    int height;
    GLint format;
    GLuint gl_name;

    GLuint last_bound_texture = NULL;
};


#endif //RENDERER_TEXTURE_H
