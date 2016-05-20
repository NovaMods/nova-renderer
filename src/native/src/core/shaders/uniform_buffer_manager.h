/*!
 * \author David
 * \date 19-May-16.
 */

#ifndef RENDERER_UNIFORM_BUFFER_MANAGER_H
#define RENDERER_UNIFORM_BUFFER_MANAGER_H

#include <string>
#include <unordered_map>
#include "../../gl/glad/glad.h"

/*!
 * \brief Holds all the uniform buffers, and their OpenGL names
 */
class uniform_buffer_manager {
public:
    /*!
     * \brief Initializes this thing, creating all the uniform buffers we need
     *
     * Something super important here: This method needs an OpenGL context, so be sure you don't call it before you
     * have one.
     */
    uniform_buffer_manager();
private:
    std::unordered_map<std::string, GLuint> uniform_buffers;
};


#endif //RENDERER_UNIFORM_BUFFER_MANAGER_H
