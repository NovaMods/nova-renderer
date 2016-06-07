/*!
 * \brief A few typedefs to improve code readability
 *
 * \author David
 * \date 26-May-16.
 */

#ifndef RENDERER_TYPES_H
#define RENDERER_TYPES_H

#include <unordered_map>
#include <string>
#include <memory>
#include "../interfaces/ishader.h"
#include "../gl/objects/gl_uniform_buffer.h"

// much easier to type this way
typedef std::unordered_map<std::string, ishader *> shader_store;
typedef std::unordered_map<std::string, gl_uniform_buffer *> uniform_buffer_store;

#endif //RENDERER_TYPES_H
