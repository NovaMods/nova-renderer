/*!
 * \brief A few typedefs to improve code readability
 *
 * \author David
 * \date 26-May-16.
 */

#ifndef RENDERER_TYPES_H
#define RENDERER_TYPES_H

#include <functional>

using geometry_filter = std::function<bool(view::render_object)>;

#endif //RENDERER_TYPES_H
