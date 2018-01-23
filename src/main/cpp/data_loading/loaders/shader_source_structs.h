/*!
 * \brief Defines a couple of structs that are super useful when loading shaders
 *
 * \author ddubois 
 * \date 18-Oct-16.
 */

#ifndef RENDERER_SHADER_SOURCE_STRUCTS_H
#define RENDERER_SHADER_SOURCE_STRUCTS_H

#include <string>
#include <memory>
#include <vector>

#include <optional.hpp>
#include <json.hpp>

#include <easylogging++.h>
#include "../../render/objects/renderpasses/materials.h"

// While I usually don't like to do this, I'm tires of typing so much
using namespace std::experimental;

namespace nova {
    /*!
     * \brief Holds a line number and file name
     *
     * This struct is used to create a map from the line of code in the shader sent to the driver and the line of
     * code on disk
     */
    struct shader_line {
        int line_num;               //!< The line number in the original source file
        std::string shader_name;    //!< The name of the original source file
        std::string line;           //!< The actual line
    };

    /*!
     * \brief Represents a shader before it goes to the GPU
     */
    struct shader_definition {
        std::string name;
        std::string filter_expression;
        optional<std::string> fallback_name;

        optional<std::shared_ptr<shader_definition>> fallback_def;

        std::vector<uint32_t> vertex_source;
        std::vector<uint32_t> fragment_source;
        // TODO: Figure out how to handle geometry and tessellation shaders

        /*!
         * \brief The framebuffer attachments that this shader writes to
         */
        std::vector<unsigned int> drawbuffers;

        explicit shader_definition(const nova::material_state &material);
    };

    struct shaderpack_definition {
        std::vector<shader_definition> shaders;
        nlohmann::json shaders_json;
    };

    el::base::Writer& operator<<(el::base::Writer& out, const std::vector<shader_line>& lines);

    el::base::Writer& operator<<(el::base::Writer& out, const shader_line& line);
}

#endif //RENDERER_SHADER_SOURCE_STRUCTS_H
