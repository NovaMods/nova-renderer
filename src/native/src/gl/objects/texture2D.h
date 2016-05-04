//
// Created by David on 26-Apr-16.
//

#ifndef RENDERER_TEXTURE_H
#define RENDERER_TEXTURE_H


#include "../glad/glad.h"
#include "../../interfaces/itexture.h"

class texture2D : public itexture {
public:
    texture2D();

    /*!
     * \brief Sets up a texture with the data from an allocator
     *
     * The idea of this is that the thing called when a resource pack is loaded will allocate all the textures ie needs,
     * then pack those allocated textures into texture2D objects so they're easier to deal with. I expect a minimum
     * number of allocations: One for the albedo texture, one for the normal texture, and one for the specular data
     * texture. I might need more, for things like particles of all the random UI textures (Those can be packed into an
     * atlas...)
     *
     * \param gl_name The OpenGl name of this texture, generated when the texture was allocated
     */
    texture2D(GLuint gl_name);

    /*!
     * \brief Binds this texture to the OpenGL context, saving the previously bound texture to an internal variable.
     *
     * For best performance, call this function once, do ABSOLUTELY EVERYTHING you need to do to this texture, then
     * unbind it
     *
     * It's worth noting that, if you give me the location that this texture is already bound to, this function won't
     * make any GL calls. For that reason, it si ABSOLUTELY IMPERATIVE that you call unbind. Always. Forever. When this
     * texture is no longer active, call unbind. Do it. You're going to forget, and you'll be really sad you can't find
     * the problem, then you're going to find the bit of code that doesn't bind the texture if it's already bound and
     * you'lll see this comment and you'll understand what you did wrong.
     *
     * \param location The location to bind this texture to
     */
    virtual void bind(unsigned int location);

    /*!
     * \brief Unbinds this texture from the OpenGL context
     *
     * This function binds texture -1 to this texture's current location. This will probably cause a lot of headaches
     * in the future, but it's pretty correct.
     */
    virtual void unbind();

    /*!
     * \brief Sets this texture's data to the given parameters
     *
     * It's worth noting that this function doesn't do any validation on its data. You specified a LDR texture format
     * but you gave me HDR data? Sure hope the GPU can deal with that
     *
     * \param pixel_data The raw pixel_data
     * \param dimensions An array of the dimensions in this texture. For a texture2D that array MUST have two elements
     * \param format The format of the texture data
     */
    virtual void set_data(std::vector<float> & pixel_data, std::vector<int> & dimensions, GLenum format);

    virtual void set_filtering_parameters(texture_filtering_params & params);

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

    const unsigned int &get_gl_name();

private:
    int width;
    int height;
    GLint format;
    GLuint gl_name;
    GLint current_location = -1;
};


#endif //RENDERER_TEXTURE_H
