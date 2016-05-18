/*!
 * \author David
 * \date 17-May-16.
 */

#ifndef RENDERER_SHADER_STORE_H
#define RENDERER_SHADER_STORE_H

#include <string>
#include <unordered_map>
#include "../../interfaces/ishader.h"

/*!
 * \brief A class to store shaders for later use
 */
class shader_store {
public:
    shader_store();

    /*!
     * \brief Gets the shader with the given name
     *
     * \param shader_name The name of the shader to aquire
     * \return The shader with the given name
     *
     * \throws std::invalid_value if the requested shader does not exist
     */
    ishader * get_shader(std::string & shader_name) const;
private:
    std::unordered_map<std::string, ishader *> shaders;
};


#endif //RENDERER_SHADER_STORE_H
