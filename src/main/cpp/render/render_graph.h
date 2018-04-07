/*!
 * \author ddubois 
 * \date 25-Feb-18.
 */

#ifndef RENDERER_RENDER_GRAPH_H
#define RENDERER_RENDER_GRAPH_H

#include "objects/renderpasses/render_passes.h"
#include "objects/textures/texture_manager.h"

namespace nova {
    /*!
     * \brief Orders the provided render passes to satisfy both their implicit and explicit dependencies
     *
     * The provided submission order may not be valid for all sets of passes. It is pretty un-optimized, preferring to
     * execute a pass earlier in the frame - so it may lead to some stalls somewhere? But I'm using OpenGL and the
     * driver deals with all that. There's things I can do that will give me much higher framerate gains than optimizing
     * the submission order
     *
     * \param passes A map from pass name to pass of all the passes to order
     * \return The names of the passes in submission order
     */
    std::vector<std::string> order_passes(const std::unordered_map<std::string, render_pass>& passes);
}

#endif //RENDERER_RENDER_GRAPH_H
