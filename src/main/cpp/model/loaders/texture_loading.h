/*!
 * \brief Defines a ton of functions to aid loading textures from a resourcepack
 *
 * \author ddubois
 * \date 05-Sep-16.
 */

#ifndef RENDERER_TEXTURE_LOADING_H
#define RENDERER_TEXTURE_LOADING_H

#include <vector>
#include <string>
#include <unordered_map>

namespace nova {
    namespace model {
        struct texture_data {
            unsigned char* data;
            int width;
            int height;
            int num_components;
        };

        //! The list of all the textures that we should load
        std::vector<std::string> textures_to_load {
                "gui/widgets.png"
        };

        /*!
         * \brief Loads all the textures that Nova cares about from the resourcepack with the provided name
         *
         * Note that the failure to load a texture does not constitute an error in thie case because, if nothing else,
         * the texture will be available in the default resourcepack
         *
         * \param resourcepack_name The name of the resourcepack to load all the textures from
         * \return A map from name to data of all the textures that can be loaded
         */
        std::unordered_map<std::string, texture_data> load_textures_from_resourcepack(const std::string& resourcepack_name);

        /*!
         * \brief Loads all the textures that Nova cares about from the resourcepack with the provided name
         *
         * It's probably worth noting that this function doesn't do anything useful if you give it the name of a regular
         * folder. It may or may not do something unuseful
         *
         * \param resourcepack_name The name of the resourcepack to load textures from
         * \return All the textures Nova cares about that were in the resourcepack
         */
        std::unordered_map<std::string, texture_data> load_textures_zip(const std::string& resourcepack_name);
    }
}

#endif //RENDERER_TEXTURE_LOADING_H
