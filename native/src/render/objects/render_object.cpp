//
// Created by ddubois on 8/8/17.
//

//#include <easylogging++.h>
#include "render_object.h"
#include "meshes/vk_mesh.h"
#include "../nova_renderer.h"

namespace nova {
    int render_object::ID;

    render_object::render_object(const std::shared_ptr<render_context> context,
                                            shader_resource_manager &shader_resources,
                                            const size_t ubo_data_size,
                                            const mesh_definition mesh_def) :
            geometry(vk_mesh(mesh_def, context)), resource_manager(shader_resources) {
        model_matrix_descriptor = shader_resources.create_model_matrix_descriptor();
        per_model_buffer_range = shader_resources.get_uniform_buffers().get_per_model_buffer()->allocate_space(ubo_data_size);
        id = ID;
        ID++;
    }

    void render_object::upload_model_matrix(const vk::Device &device) const {
        // Copy the memory to the descriptor set
        auto write_ds = vk::WriteDescriptorSet()
                .setDstSet(model_matrix_descriptor)
                .setDstBinding(0)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setPBufferInfo((&per_model_buffer_range));

        device.updateDescriptorSets({write_ds}, {});
    }

    render_object::~render_object() {
        if (model_matrix_descriptor != vk::DescriptorSet()) {
            resource_manager.get_uniform_buffers().get_per_model_buffer()->free_allocation(per_model_buffer_range);
            resource_manager.free_descriptor(model_matrix_descriptor);
            model_matrix_descriptor = nullptr;
        }
    }

    render_object::render_object(render_object &&other) :
            parent_id(other.parent_id),
            type(other.type),
            geometry(std::move(other.geometry)),
            position(other.position),
            id(other.id),
            model_matrix_descriptor(other.model_matrix_descriptor),
            per_model_buffer_range(other.per_model_buffer_range),
            resource_manager(other.resource_manager) {
        // make the destructor of other not free vulkan resources
        other.model_matrix_descriptor = nullptr;
    }

    render_object& render_object::operator=(render_object &&other) noexcept {
        if (this == &other) {
            return *this;
        }

        if (model_matrix_descriptor != vk::DescriptorSet()) {
            // free stuff from this if not already moved from
            resource_manager.get_uniform_buffers().get_per_model_buffer()->free_allocation(per_model_buffer_range);
            resource_manager.free_descriptor(model_matrix_descriptor);
        }
        // copy data
        parent_id = other.parent_id;
        type = other.type;
        geometry = std::move(other.geometry);
        position = other.position;
        id = other.id;
        model_matrix_descriptor = other.model_matrix_descriptor;
        per_model_buffer_range = other.per_model_buffer_range;
        resource_manager = other.resource_manager;

        // set other to moved from state
        other.model_matrix_descriptor = nullptr;
        return *this;
    }

    render_object::render_object() : resource_manager{*nova_renderer::instance->get_shader_resources()} {
    }
}