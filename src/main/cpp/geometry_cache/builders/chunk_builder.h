/*!
 * \brief
 *
 * \author ddubois 
 * \date 02-Mar-17.
 */

#ifndef RENDERER_CHUNK_BUILDER_H
#define RENDERER_CHUNK_BUILDER_H

#include <string>
#include <vector>
#include <optional.hpp>
#include "../../mc_interface/mc_objects.h"
#include "../../render/objects/shaders/shaderpack.h"

using namespace std::experimental;

namespace nova {
    /*!
     * \brief Builds all the render objects needed to render the provided chunk with the provided shaderpack
     *
     * One render_object is made for each shader in the shaderpack (unless no geometry in the chunk should be rendered
     * by the shaders, in which case there's no render_objects made). Each render_object has all the geometry that
     * should be rendered by the provided shader given the shader's filters
     *
     * \param chunk The chunk to process
     * \param shaders The shaderpack that will be used to render this chunk
     * \return A map from name of shader to render_object made for that shader
     */
    std::unordered_map<std::string, optional<render_object>> get_renderables_from_chunk(mc_chunk chunk, shaderpack& shaders);

    /*!
     * \brief Creates a render_object that matches the provided filter
     *
     * \param chunk The chunk to build the render_object from
     * \param filter The filter that decides which geometry to select for rendering
     * \return A render_object which holds all the geometry to be rendered by the provided filter, or an empty optional
     * if nothing matches the filter
     */
    optional<render_object> &build_render_object_for_shader(mc_chunk chunk, geometry_filter filter);
}


#endif //RENDERER_CHUNK_BUILDER_H
