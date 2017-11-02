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
    /*!
     * \brief Loads the shaderpack with the given name
     *
     * This function should build the renderpass when when loading the shaderpack, not have the renderpass passed in
     *
     * \param shaderpack_name The name of the shaderpack to load
     * \param parent_renderpass The renderpass that this shaderpack belongs to. Future versions will be more intelligent
     * \return The loaded shaderpack
     */
    shaderpack load_shaderpack(const std::string &shaderpack_name, std::shared_ptr<renderpass> parent_renderpass);
}

#endif //RENDERER_LOADERS_H
