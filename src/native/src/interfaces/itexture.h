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
    /*!
     * \brief Allocates a new texture on the GPU
     *
     * \throws texture_creation_exception if the texture can't be created for some reason
     */
    virtual void create() = 0;

    /*!
     * \brief Deletes this texture from the GPU
     */
    virtual void destroy() = 0;

    /*!
     * \brief Tells the graphics context to use this texture for the given texture slot
     *
     * TODO: Vulkan may or may not use texture slots like OpenGL does. Probably not. WIll consider later.
     *
     * \param location The identifier to bind this texture to
     *
     * \throws texture_bind_exception if the texture can't be bound
     */
    virtual void bind(unsigned int location) = 0;

    /*!
     * \brief Tells the graphics context to not use this texture any more
     *
     * Not 100% sure how I'll handle this. I'll probably figure it out when I code it. Hopefully before, though
     */
    virtual void unbind() = 0;

    /*!
     * \brief Sets the data for this texture
     *
     * This method should upload the texture data to the GPU ASAP and set some internal variables to save the result of
     * the texture data uploading
     *
     * \param pixel_data The raw pixel data for this texture
     * \param height The height, in pixels, of this texture
     * \param width The width, in pixels, of this texture
     * \param num_channels How many color channels the texture has
     */
    virtual void set_data(std::vector<float> pixel_data, int height, int width, int num_channels) = 0;

    /*!
     * \brief Sets this texture's filtering parameters
     *
     * \param params The filtering parameters for this texture
     */
    virtual void set_filtering_params(texture_filtering_params params) = 0;
};


#endif //RENDERER_ITEXTURE_H
