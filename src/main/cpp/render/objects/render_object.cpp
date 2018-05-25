//
// Created by ddubois on 8/8/17.
//

#include <easylogging++.h>
#include "render_object.h"
#include "meshes/vk_mesh.h"

namespace nova {
    int render_object::ID;

    render_object::render_object() {
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
}