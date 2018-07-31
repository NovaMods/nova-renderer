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
#include "../../physics/aabb.h"
#include "resources/shader_resource_manager.h"
#include "meshes/mesh_definition.h"
#include "meshes/vk_mesh.h"

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
     * \brief Represents something that can be rendered
     *
     * This provides a number of values that you can filter things by.
     */
    class render_object {
    public:
        int parent_id = -1;  //!< The ID of the thing that owns us. Could be the ID of a chunk, entity, whatever

        geometry_type type;

        vk_mesh geometry;

        glm::vec3 position;

        aabb bounding_box;

        int id = -1;

        // TODO: make it private
        vk::DescriptorSet model_matrix_descriptor;

        render_object();

        render_object(const std::shared_ptr<render_context> context, shader_resource_manager &shader_resources, const size_t ubo_data_size, const mesh_definition mesh_def);

        ~render_object();

        render_object(render_object&& other);

        render_object& operator=(render_object&& other) noexcept;

        void upload_model_matrix(const vk::Device& device) const;

        template <class per_model_uniform>
        void write_new_model_ubo(const per_model_uniform &model_ubo) const {
            auto& allocation = resource_manager.get_uniform_buffers().get_per_model_buffer()->get_allocation_info();
            memcpy(((uint8_t*)allocation.pMappedData) + per_model_buffer_range.offset, reinterpret_cast<const void*>(&model_ubo), per_model_buffer_range.range);
        }

    private:
        vk::DescriptorBufferInfo per_model_buffer_range;
        shader_resource_manager &resource_manager;

        static int ID;
    };
}

#endif //RENDERER_RENDER_OBJECT_H
