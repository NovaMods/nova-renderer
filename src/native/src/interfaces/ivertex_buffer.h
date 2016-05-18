//
// Created by David on 14-Jan-16.
//

#ifndef RENDERER_IVERTEX_BUFFER_H
#define RENDERER_IVERTEX_BUFFER_H

#include <vector>

/*!
 * \brief Interface to abstract away vertex buffers between GL and Vulkan
 *
 * Defines a few methods to operate on vertex buffers to do the basic things the Vulkan mod needs. OpenGL and Vulkan
 * have separate implementations of vertex buffers, as they should
 */
class ivertex_buffer {
public:
    /*!
     * \brief Specifies the format of vertex buffer data
     */
    enum class format {
        /*!
         * \brief The vertex buffer only has positional data (this is useful somehow)
         */
        POS,

        /*!
         * \brief The vertex buffer has positions and texture coordinates (Like the sun and particles, I think)
         */
        POS_UV,

        /*!
         * \brief The vertex buffer has positions, texture coordinates, and normals (Terrain and entities)
         */
        POS_NORMAL_UV
    };

    /*!
     * \brief Specifies how the data in thie buffer will be used
     */
    enum class usage {
        /*!
         * \brief The buffer will be updated once and drawn many times
         */
        static_draw,

        /*!
         * \brief The buffer will be updated many times and drawn many times
         */
        dynamic_draw,
    };

    /*!
     * \brief Creates a new Vertex Buffer, allocating resources on the GPU for this vertex buffer to live in
     */
    virtual void create() = 0;

    /*!
     * \brief Destroys this vertex buffer on the GPU, freeing up VRAM for other things
     */
    virtual void destroy() = 0;

    /*!
     * \brief Sets the given data as this vertex buffer's data, and uploads that data to the GPU
     *
     * \param data The interleaved vertex data
     * \param data_format The format of the data (\see format)
     */
    virtual void set_data(std::vector<float> data, format data_format, usage data_usage);

    /*!
     * \brief Sets the index array for this vertex buffer, so that anything using it knows how to handle itself
     */
    virtual void set_index_array(std::vector<unsigned short> data, usage data_usage) = 0;

    /*!
     * \brief Sets this vertex buffer as the one currently being drawn, allowing it to actually be drawn
     */
    virtual void set_active() = 0;

    /*!
     * \brief Draws the
     */
    virtual void draw() = 0;

    /*!
     * \brief Returns the format of this vertex buffer
     *
     * \return The format of this vertex buffer
     */
    virtual format get_format();

protected:
    format data_format;
};


#endif //RENDERER_IVERTEX_BUFFER_H
