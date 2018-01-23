/*!
 * \brief
 *
 * \author ddubois 
 * \date 20-Sep-16.
 */

#ifndef RENDERER_SHADER_INTERFACE_H
#define RENDERER_SHADER_INTERFACE_H

#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <initializer_list>
#include <mutex>
#include <optional.hpp>

#include "gl_shader_program.h"
#include "../../../data_loading/loaders/shader_source_structs.h"

namespace nova {
    /*!
     * \brief Represents a shaderpack in all of its glory, along with some meta information about the options that this
     * shaderpack sets
     */
    class shaderpack {
    public:
        /*!
         * \brief Loads the shaderpack with the given name
         *
         * This is kinda gross because the shaderpack loading logic is all
         * in the data_loading module... thing is, there's no longer any
         * reason to keep that running in a separate thread, so why not put
         * it here?
         *
         * \param shaderpack_name The name of the shaderpcack to load
         *
         */
        shaderpack(std::string name, nlohmann::json shaders_json, std::vector<shader_definition> &shaders);

        gl_shader_program &operator[](std::string key);

        gl_shader_program& get_shader(std::string key);

		std::unordered_map<std::string, gl_shader_program> &get_loaded_shaders();

        void operator=(const shaderpack& other);

        std::string& get_name();

    private:
        std::unordered_map<std::string, gl_shader_program> loaded_shaders;

        std::string name;

        /*!
         * \brief The indices of the framebuffer attachments that any of the non-shadow shaders write to
         */
        std::vector<unsigned int> all_drawbuffers;

        /*!
         * \brief The indices of all the framebuffer attachments that any of the shadow shaders write to
         */
        std::vector<unsigned int> shadow_drawbuffers;

        /*!
         * \brief The options that the shaders in this shaderpack set
         */
        nlohmann::json options;
    };
}

#endif //RENDERER_SHADER_INTERFACE_H.
