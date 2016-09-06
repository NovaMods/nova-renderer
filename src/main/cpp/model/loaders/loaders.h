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

#include "../gl/gl_shader_program.h"

namespace nova {
    namespace model {
        /*!
         * \brief Loads the shaderpack with the given name
         *
         * \param shaderpack_name The name of the shaderpack to load
         * \return The loaded shaderpack
         */
        std::unordered_map<std::string, gl_shader_program> load_shaderpack(const std::string& shaderpack_name);

        /*!
         * \brief Loads all the textures in all the provided resourcepacks
         *
         * Note that if a resourcepack name starts with "/", then the resourcepack is assumed to live at the root of the
         * .minecraft folder. Otherwise, the resourcepack is assumed to live in the resourecepacks folder
         *
         * Note also that textures are loaded from the resourcepacks in the order that the resourcepacks are specified,
         * and that resouresepacks listed later in the vector cannot overwrite data from resourcepacks earlier in the
         * vector
         *
         * \param resourcepack_names The names of the resource packs to load
         * \return A map from string name to OpenGL name of all the loaded textures
         */
        std::unordered_map<std::string, GLuint> load_textures(const std::vector<std::string>& resourcepack_names);
    }
}

#endif //RENDERER_LOADERS_H
