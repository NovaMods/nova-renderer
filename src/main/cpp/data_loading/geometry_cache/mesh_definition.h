/*!
 * \brief
 *
 * \author ddubois 
 * \date 14-Nov-16.
 */

#ifndef RENDERER_MESH_DEFINITION_H
#define RENDERER_MESH_DEFINITION_H

#include <vector>

namespace nova {
    /*!
     * \brief Specifies the format of vertex buffer data
     */
    enum class format {
        /*!
         * \brief The vertex buffer only has positional data (this is useful somehow)
         */
                POS,

        /*!
         * \brief The vertex buffer has positions and texture coordinates (Like the sun and particles, I think)
         */
                POS_UV,

        /*!
         * \brief The vertex buffer has positions, texture coordinates, and normals (Terrain and entities)
         */
                POS_UV_LIGHTMAPUV_NORMAL_TANGENT
    };

    /*!
     * \brief Defines the geometry in a mesh so that you can just throw the mesh onto the GPU and not care
     */
    struct mesh_definition {
        std::vector<float> vertex_data;
        std::vector<unsigned> indices;
        format vertex_format;
    };
}

#endif //RENDERER_MESH_DEFINITION_H
