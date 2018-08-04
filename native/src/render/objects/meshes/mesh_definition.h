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
    SMART_ENUM(geometry_type, \
    block, \
    entity, \
    falling_block, \
    gui, \
    text, \
    gui_background, \
    cloud, \
    sky_decoration, \
    selection_box, \
    glint, \
    weather, \
    hand, \
    fullscreen_quad, \
    particle, \
    lit_particle, \
    eyes)

    /*!
     * \brief Specifies the format of vertex buffer data
     */
    SMART_ENUM(format, \
        POS, \
        POS_UV, \
        POS_COLOR_UV_LIGHTMAPUV_NORMAL_TANGENT, \
        POS_UV_COLOR)

    struct nova_vertex {
        glm::vec3 position;
        glm::vec2 uv0;
        glm::vec2 mid_tex_coord;
        int virtual_texture_id;
        glm::vec4 color;
        glm::vec2 uv1;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec4 metadata;
    };

    /*!
     * \brief Defines the geometry in a mesh so that you can just throw the mesh onto the GPU and not care
     */
    struct mesh_definition {
        std::vector<nova_vertex> vertex_data;
        std::vector<int> indices;
        format vertex_format;
        glm::vec3 position;
        int id;

        geometry_type type;
    };
}

#endif //RENDERER_MESH_DEFINITION_H
