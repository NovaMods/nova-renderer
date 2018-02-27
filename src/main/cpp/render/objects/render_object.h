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
#include <optional.hpp>

#include "gl_mesh.h"
#include "../../utils/smart_enum.h"
#include "textures/texture_manager.h"


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
    eyes);

    /*!
     * \brief Represents something that can be rendered
     *
     * This provides a number of values that you can filter things by.
     */
    struct render_object {
        int parent_id;  //!< The ID of the thing that owns us. Could be the ID of a chunk, entity, whatever

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

        std::unique_ptr<gl_mesh> geometry;

        std::string color_texture;
        std::experimental::optional<std::string> normalmap;
        std::experimental::optional<std::string> data_texture;

        glm::vec3 position;

        aabb bounding_box;

        bool needs_deletion;

        render_object() = default;
        render_object(render_object&& other) noexcept;
        render_object(const render_object&) = default;

        render_object& operator=(render_object&& other) noexcept;
    };
}

#endif //RENDERER_RENDER_OBJECT_H
