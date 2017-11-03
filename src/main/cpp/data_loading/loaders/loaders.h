/*!
 * \brief Defines functions to load the data that Nova needs to load
 *
 * This file is intended as the public interface of the data loading module
 *
 * \author ddubois 
 * \date 03-Sep-16.
 */

#ifndef RENDERER_LOADERS_H
#define RENDERER_LOADERS_H

#include <string>
#include <unordered_map>
#include "../../render/objects/shaders/shaderpack.h"

namespace nova {
    class shader_definition;

    /*!
     * \brief Loads the shaderpack with the given name
     *
     * This function should build the renderpass when when loading the shaderpack, not have the renderpass passed in
     *
     * TODO: Make this fucntion return a shaderpack definition, which will ahve all the sahder definitions and the
     * material definitions
     *
     * \param shaderpack_name The name of the shaderpack to load
     * \return The loaded shaderpack
     */
    std::vector<shader_definition> load_shaderpack(const std::string &shaderpack_name);
}

#endif //RENDERER_LOADERS_H
