/*!
 * \brief Defines a couple of structs that are super useful when loading shaders
 *
 * \author ddubois 
 * \date 18-Oct-16.
 */

#ifndef RENDERER_SHADER_SOURCE_STRUCTS_H
#define RENDERER_SHADER_SOURCE_STRUCTS_H

#include <string>
#include <vector>

namespace nova {
    namespace model {
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

        struct shader_source {
            std::vector<shader_line> vertex_source;
            std::vector<shader_line> fragment_source;
            // TODO: Figure out how to handle geometry and tessellation shaders
        };
    }
}

#endif //RENDERER_SHADER_SOURCE_STRUCTS_H
