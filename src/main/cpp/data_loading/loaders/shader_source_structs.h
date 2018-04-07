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
#include <experimental/filesystem>

#include <optional.hpp>
#include <json.hpp>

#include <easylogging++.h>
#include "../../utils/smart_enum.h"

// While I usually don't like to do this, I'm tires of typing so much
using namespace std::experimental;

namespace fs = std::experimental::filesystem;

namespace nova {
    SMART_ENUM(shader_langauge_enum,
        SPIRV,
        GLSL,
        GLSLES,
        HLSL,
    )

    /*!
     * \brief Holds a line number and file name
     *
     * This struct is used to create a map from the line of code in the shader sent to the driver and the line of
     * code on disk
     */
    struct shader_line {
        int line_num;               //!< The line number in the original source file
        fs::path shader_name;    //!< The name of the original source file
        std::string line;           //!< The actual line
    };

    struct shader_file {
        std::vector<shader_line> lines;
        shader_langauge_enum language;
    };

    /*!
     * \brief Represents a shader before it goes to the GPU
     */
    struct shader_definition {
        optional<std::string> fallback_name;

        optional<std::shared_ptr<shader_definition>> fallback_def;

        shader_file vertex_source;
        shader_file fragment_source;
        optional<shader_file> geometry_source;
        optional<shader_file> tessellation_control_source;
        optional<shader_file> tessellation_evaluation_source;
    };

    el::base::Writer& operator<<(el::base::Writer& out, const std::vector<shader_line>& lines);

    el::base::Writer& operator<<(el::base::Writer& out, const shader_line& line);

    shader_langauge_enum language_from_extension(const std::string& extension);
}

#endif //RENDERER_SHADER_SOURCE_STRUCTS_H
