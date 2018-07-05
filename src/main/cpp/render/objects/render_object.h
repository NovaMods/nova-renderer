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

#include "../../utils/smart_enum.h"
#include "resources/texture_manager.h"
#include "../../physics/aabb.h"


namespace nova {
    class vk_mesh;

    SMART_ENUM(geometry_type, \
    block, \
    entity, \
    falling_block, \
    gui, \
    text, \
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
        int parent_id;  //!< The ID of the thing that owns us. Could be the ID of a chunk, entity, whatever

        geometry_type type;

        std::shared_ptr<vk_mesh> geometry;

        glm::vec3 position;

        aabb bounding_box;

        vk::DescriptorSet model_matrix_descriptor;
        vk::DescriptorBufferInfo per_model_buffer_range;

        static int ID;
        int id;

        render_object();

        void upload_model_matrix(const vk::Device& device) const;
    };
}

#endif //RENDERER_RENDER_OBJECT_H
