/*!
 * \brief
 *
 * \author ddubois 
 * \date 14-Nov-16.
 */

#ifndef RENDERER_MESH_DEFINITION_H
#define RENDERER_MESH_DEFINITION_H

#include <vector>
#include <glm/glm.hpp>
#include "../../../utils/smart_enum.h"

namespace nova {
    /*!
     * \brief Specifies the format of vertex buffer data
     */
    SMART_ENUM(format, \
        POS, \
        POS_UV, \
        POS_COLOR_UV_LIGHTMAPUV_NORMAL_TANGENT, \
        POS_UV_COLOR)

    /*!
     * \brief Defines the geometry in a mesh so that you can just throw the mesh onto the GPU and not care
     */
    struct mesh_definition {
        std::vector<int> vertex_data;
        std::vector<int> indices;
        format vertex_format;
        glm::vec3 position;
        int id;
    };
}

#endif //RENDERER_MESH_DEFINITION_H
