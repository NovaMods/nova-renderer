/*!
 * \author David
 * \date 17-May-16.
 */

#ifndef RENDERER_ISHADER_H
#define RENDERER_ISHADER_H

#include <string>

/*!
 * \brief Represents a shader object in an API-agnostic way
 */
class ishader {
public:
    /*!
     * \brief Sets this shader as the currently active shader
     */
    virtual void bind() noexcept = 0;

    /*!
     * \brief Gets the locaiton of the given uniform variable
     *
     * This is kinda GL-specific, but I'm only using OpenGL so I'm not all that worried at this point
     *
     * \param uniform_name The name of the uniform variable to get
     * \return The OpenGL location of the given uniform
     *
     * \throws std::invalid_value if the given uniform name isn't in this shader
     */
    virtual int get_uniform_location(std::string & uniform_name) const = 0;

    /*!
     * \brief Gets the locaiton of the given attribute variable
     *
     * This is kinda GL-specific, but I'm only using OpenGL so I'm not all that worried at this point
     *
     * \param uniform_name The name of the attribute variable to get
     * \return The OpenGL location of the given attribute
     *
     * \throws std::invalid_value if the given attribute name isn't in this shader
     */
    virtual int get_attribute_location(std::string & attribute_name) const = 0;

    /*!
     * \brief Sets the given integer as the data for the uniform variable with the given location
     *
     * \param location The uniform location to put the data in
     * \param data The data to give to the given uniform location
     */
    virtual void set_uniform_data(unsigned int location, int data) noexcept = 0;
};

#endif //RENDERER_ISHADER_H
