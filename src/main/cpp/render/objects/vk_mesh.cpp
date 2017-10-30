/*!
 * \author David
 * \date 13-May-16.
 */

#include <stdexcept>
#include <easylogging++.h>
#include "vk_mesh.h"
#include "../windowing/glfw_vk_window.h"
#include "../vulkan/render_context.h"

namespace nova {
    vk_mesh::vk_mesh() : num_indices(0) {
    }

    vk_mesh::vk_mesh(const mesh_definition &definition) {
        set_data(definition.vertex_data, definition.vertex_format, definition.indices);
    }

    vk_mesh::~vk_mesh() {
        destroy();
    }

    void vk_mesh::destroy() {
        if((VkBuffer)vertex_buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(render_context::instance.allocator, vertex_buffer, vertex_alloc);
        }

        if((VkBuffer)indices != VK_NULL_HANDLE) {
            vmaDestroyBuffer(render_context::instance.allocator, indices, indices_alloc);
        }
    }

    void vk_mesh::set_data(const std::vector<int>& vertex_data, const format data_format, const std::vector<int>& index_data) {
        auto& context = render_context::instance;
        this->data_format = data_format;

        upload_vertex_data(vertex_data, context);
        upload_index_data(index_data, context);

        enable_vertex_attributes(data_format);
    }

    void vk_mesh::set_active(vk::CommandBuffer command) const {
        vk::DeviceSize offset = 0;
        command.bindVertexBuffers(0, 1, &vertex_buffer, &offset);
    }

    void vk_mesh::draw() const {
        //glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr);
    }

    void vk_mesh::enable_vertex_attributes(format data_format) {
        switch(data_format) {
            case format::POS:
                // Location in shader, buffer binding, data format, offset in buffer
                attribute_descriptions.emplace_back(0, 0, vk::Format::eR32G32B32Sfloat, 0);     // Position

                // Binding, stride, input rate
                binding_descriptions.emplace_back(0, 12, vk::VertexInputRate::eVertex);         // Position

                break;

            case format::POS_UV:
                // Location in shader, buffer binding, data format, offset in buffer
                attribute_descriptions.emplace_back(0, 0, vk::Format::eR32G32B32Sfloat, 0);     // Position
                attribute_descriptions.emplace_back(1, 0, vk::Format::eR32G32Sfloat,    12);    // UV

                // Binding, stride, input rate
                binding_descriptions.emplace_back(0, 20, vk::VertexInputRate::eVertex);         // Position
                binding_descriptions.emplace_back(1, 20, vk::VertexInputRate::eVertex);         // UV

                break;

            case format::POS_UV_COLOR:
                // Location in shader, buffer binding, data format, offset in buffer
                attribute_descriptions.emplace_back(0, 0, vk::Format::eR32G32B32Sfloat,    0);  // Position
                attribute_descriptions.emplace_back(1, 0, vk::Format::eR32G32Sfloat,       12); // UV
                attribute_descriptions.emplace_back(2, 0, vk::Format::eR32G32B32A32Sfloat, 20); // Color

                // Binding, stride, input rate
                binding_descriptions.emplace_back(0, 36, vk::VertexInputRate::eVertex);         // Position
                binding_descriptions.emplace_back(1, 36, vk::VertexInputRate::eVertex);         // UV
                binding_descriptions.emplace_back(2, 36, vk::VertexInputRate::eVertex);         // Color

                break;

            case format::POS_COLOR_UV_LIGHTMAPUV_NORMAL_TANGENT:
                // Location in shader, buffer binding, data format, offset in buffer
                attribute_descriptions.emplace_back(0, 0, vk::Format::eR32G32B32Sfloat, 0);     // Position
                attribute_descriptions.emplace_back(1, 0, vk::Format::eR8G8B8A8Unorm,   12);    // Color
                attribute_descriptions.emplace_back(2, 0, vk::Format::eR32G32Sfloat,    16);    // UV
                attribute_descriptions.emplace_back(3, 0, vk::Format::eR16G16Unorm,     24);    // Lightmap UV
                attribute_descriptions.emplace_back(4, 0, vk::Format::eR32G32B32Sfloat, 32);    // Normal
                attribute_descriptions.emplace_back(5, 0, vk::Format::eR32G32B32Sfloat, 48);    // Tangent

                // Binding, stride, input rate
                binding_descriptions.emplace_back(0, 56, vk::VertexInputRate::eVertex);         // Position
                binding_descriptions.emplace_back(1, 56, vk::VertexInputRate::eVertex);         // Color
                binding_descriptions.emplace_back(2, 56, vk::VertexInputRate::eVertex);         // UV
                binding_descriptions.emplace_back(3, 56, vk::VertexInputRate::eVertex);         // Lightmap
                binding_descriptions.emplace_back(4, 56, vk::VertexInputRate::eVertex);         // Normal
                binding_descriptions.emplace_back(5, 56, vk::VertexInputRate::eVertex);         // Tangent

                break;
        }
    }

    format vk_mesh::get_format() {
        return data_format;
    }

    bool vk_mesh::has_data() const {
        return num_indices > 0;
    }

    void vk_mesh::upload_vertex_data(const std::vector<int> &vertex_data, const render_context &context) {
        vk::BufferCreateInfo vertex_buffer_create = {};
        vertex_buffer_create.size = vertex_data.size() * sizeof(int);
        vertex_buffer_create.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
        vertex_buffer_create.sharingMode = vk::SharingMode::eExclusive;
        vertex_buffer_create.queueFamilyIndexCount = 1;
        vertex_buffer_create.pQueueFamilyIndices = &context.graphics_family_idx;

        VmaAllocationCreateInfo vertex_buffer_alloc_create_info = {};
        vertex_buffer_alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        vmaCreateBuffer(context.allocator,
                        reinterpret_cast<VkBufferCreateInfo*>(&vertex_buffer_create), &vertex_buffer_alloc_create_info,
                        reinterpret_cast<VkBuffer*>(&vertex_buffer), &vertex_alloc, nullptr);

        void* mapped_vbo;
        vmaMapMemory(context.allocator, vertex_alloc, &mapped_vbo);
        memcpy(mapped_vbo, vertex_data.data(), vertex_buffer_create.size);
        vmaUnmapMemory(context.allocator, vertex_alloc);
    }

    void vk_mesh::upload_index_data(const std::vector<int>& index_data, const render_context &context) {
        vk::BufferCreateInfo index_buffer_create = {};
        index_buffer_create.size = index_data.size() * sizeof(int);
        index_buffer_create.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
        index_buffer_create.sharingMode = vk::SharingMode::eExclusive;
        index_buffer_create.queueFamilyIndexCount = 1;
        index_buffer_create.pQueueFamilyIndices = &context.graphics_family_idx;

        VmaAllocationCreateInfo index_buffer_alloc_create_info = {};
        index_buffer_alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        vmaCreateBuffer(context.allocator,
                        reinterpret_cast<VkBufferCreateInfo*>(&index_buffer_create), &index_buffer_alloc_create_info,
                        reinterpret_cast<VkBuffer*>(&indices), &indices_alloc, nullptr);

        void* mapped_vbo;
        vmaMapMemory(context.allocator, indices_alloc, &mapped_vbo);
        memcpy(mapped_vbo, index_data.data(), index_buffer_create.size);
        vmaUnmapMemory(context.allocator, indices_alloc);

        num_indices = static_cast<uint32_t>(index_data.size());
    }

    std::vector<vk::VertexInputBindingDescription> &vk_mesh::get_binding_descriptions() {
        return binding_descriptions;
    }

    std::vector<vk::VertexInputAttributeDescription> &vk_mesh::get_attribute_descriptions() {
        return attribute_descriptions;
    }
}
