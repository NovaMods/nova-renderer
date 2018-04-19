//
// Created by ddubois on 8/8/17.
//

#include "render_object.h"
#include "meshes/vk_mesh.h"

namespace nova {
    render_object::render_object(render_object &&other) noexcept :
            parent_id(other.parent_id), type(other.type), geometry(std::move(other.geometry)),
            per_model_buffer_range(other.per_model_buffer_range)
    {

        other.parent_id = 0;
        other.geometry.reset();
        other.position = {0, 0, 0};
        other.per_model_buffer_range = vk::DescriptorBufferInfo{};
    }

    render_object &render_object::operator=(render_object && other) noexcept {
        parent_id = other.parent_id;
        type = other.type;
        geometry = std::move(other.geometry);
        position = other.position;
        per_model_buffer_range = other.per_model_buffer_range;

        other.parent_id = 0;
        other.geometry.reset();
        other.position = {0, 0, 0};
        other.per_model_buffer_range = vk::DescriptorBufferInfo{};

        return *this;
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

        device.updateDescriptorSets(1, &write_ds, 0, nullptr);
    }
}