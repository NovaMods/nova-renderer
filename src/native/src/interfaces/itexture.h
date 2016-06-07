//
// Created by David on 15-Jan-16.
//

#ifndef RENDERER_ITEXTURE_H
#define RENDERER_ITEXTURE_H

#include <exception>
#include <vector>

/*!
 * \brief Encapsulates the parameters for texture filtering
 */
class texture_filtering_params {
public:
    enum filter {
        POINT,
        BILINEAR,
        TRILINEAR,
    };

    filter texture_upsample_filter;
    filter texture_downsample_filter;

    int num_mipmap_levels;
    int anisotropic_level;
};

class texture_creation_exception : public std::exception {};
class texture_bind_exception : public std::exception {};

/*!
 * \brief Interface to abstract away texture creation, deletion, and binding
 *
 * Allows me to create an opengl_texture and vulkan_texture class to handle textures in OpenGL- and Vulkan-specific ways
 * while still having a nice unified texture interface.
 *
 * I anticipate needing a texture factory of some sort. My texture objects are going to need to interact with the
 * graphics card, so I can pass my "Vulkan GPU interface" or "OpenGL GPU interface" in, assuming said interface is
 * stateful. If it isn't, I can just include the appropriate GPU interface in the Vulkan and OpenGL texture
 * implementations without too much hassle. It all depends on how stateful the GPU interfaces are.
 */
class itexture {
public:
    /*!
     * \brief Tells the graphics context to use this texture for the given texture slot
     *
     * \param location The identifier to bind this texture to
     *
     * \throws texture_bind_exception if the texture can't be bound
     */
    virtual void bind(unsigned int location) = 0;

    /*!
     * \brief Tells the graphics context to not use this texture any more, essentially making the texture slot it was
     * bound to empty
     */
    virtual void unbind() = 0;

    /*!
     * \brief Sets the data for this texture
     *
     * This method should upload the texture data to the GPU ASAP and set some internal variables to save the result of
     * the texture data uploading
     *
     * \param pixel_data The raw pixel_data
     * \param dimensions An array of the dimensions in this texture. For a texture2D that array MUST have two elements
     * \param format The format of the texture data
     */
    virtual void set_data(std::vector<float> & pixel_data, std::vector<int> & dimensions, GLenum num_channels) = 0;

    /*!
     * \brief Sets this texture's filtering parameters
     *
     * \param params The filtering parameters for this texture
     */
    virtual void set_filtering_parameters(texture_filtering_params & params) = 0;
};


#endif //RENDERER_ITEXTURE_H
