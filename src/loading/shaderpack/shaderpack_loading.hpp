/*!
 * \author ddubois
 * \date 21-Aug-18.
 */

#ifndef NOVA_RENDERER_SHADERPACK_LOADING_HPP
#define NOVA_RENDERER_SHADERPACK_LOADING_HPP

#include "nova_renderer/shaderpack_data.hpp"
#include "nova_renderer/util/filesystem.hpp"


namespace nova::renderer::shaderpack {
    /*!
     * \brief Loads all the data for a single shaderpack
     *
     * This function reads the shaderpack data from disk (either a folder od a zip file) and performs basic validation
     * to ensure both that the data is well-formatted JSON, but also to ensure that the data has all the fields that
     * Nova requires, e.g. a material must have at least one pass, a texture must have a width and a height, etc. All
     * generated warnings and errors are printed to the Nova logger
     *
     * If the shaderpack can't be loaded, an empty optional is returned
     *
     * Note: This function is NOT thread-safe. It should only be called for a single thread at a time
     *
     * \param shaderpack_name The name of the shaderpack to loads
     * \return The shaderpack, if it can be loaded, or an empty optional if it cannot
     */
    ShaderpackData load_shaderpack_data(const fs::path& shaderpack_name);
} // namespace nova::renderer

#endif // NOVA_RENDERER_SHADERPACK_LOADING_HPP
