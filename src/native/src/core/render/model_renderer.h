/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_MODEL_RENDERER_H
#define RENDERER_MODEL_RENDERER_H

/*!
 * \brief A namepsace to hold the code to drawing vertex buffers.
 *
 * It doesn't deal with shaders, or with matrices, or anything else. It simply draws vertex buffers.
 *
 * Why make so simple a file? Mostly so I don't have to re-write the vertex buffer drawing logic across the code base
 */
#include "../../interfaces/ivertex_buffer.h"

namespace model_renderer {
void draw_vertex_buffer(ivertex_buffer & buffer_to_draw) const;
};


#endif //RENDERER_MODEL_RENDERER_H
