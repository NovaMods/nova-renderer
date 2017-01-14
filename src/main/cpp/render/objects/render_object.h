/*!
 * \brief
 *
 * \author ddubois 
 * \date 20-Sep-16.
 */

#ifndef RENDERER_RENDER_OBJECT_H
#define RENDERER_RENDER_OBJECT_H

#include <string>
#include <memory>

#include "gl_mesh.h"
#include "../../utils/smart_enum.h"


namespace nova {
    SMART_ENUM(geometry_type, \
    block, \
    entity, \
    falling_block, \
    gui, \
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
     * \brief Represents something that can be rendered
     *
     * This provides a number of values that you can filter things by.
     */
    struct render_object {
        bool is_solid;
        bool is_transparent;
        bool is_cutout;
        bool is_emissive;
        unsigned char damage_level;

        geometry_type type;

        /*!
         * \brief The name of this render object
         *
         * Can have the following values:
         * - <The name of a block>
         * - <The name of an entity>
         * - gui
         * - cloud
         * - selection_box
         * - <The name of a particle system>
         * - world_border
         * - sky
         * - horizon
         * - stars
         * - void
         * - sun
         * - moon
         * - glint
         * - eyes
         * - hand
         * - rain
         * - snow
         * - fullscreen_quad
         */
        std::string name;

        gl_mesh *geometry;
    };
}

#endif //RENDERER_RENDER_OBJECT_H
