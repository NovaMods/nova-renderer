/*!
 * \author David
 * \date 13-May-16.
 */

#include <stdexcept>
#include <easylogging++.h>
#include "vk_mesh.h"
#include "../../windowing/glfw_vk_window.h"
#include "../../vulkan/render_context.h"
#include "../../nova_renderer.h"

namespace nova {
    vk_mesh::vk_mesh(std::shared_ptr<render_context> context) : num_indices(0), context(context) {
    }

    vk_mesh::vk_mesh(const mesh_definition &definition, std::shared_ptr<render_context> context) : context(context) {
        set_data(definition.vertex_data, definition.indices);
    }

    vk_mesh::~vk_mesh() {
        destroy();
    }

    void vk_mesh::destroy() {
        auto allocator = context->allocator;
        if((VkBuffer)vertex_buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator, (VkBuffer)vertex_buffer, vertex_alloc);
        }

        if((VkBuffer)indices != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator, (VkBuffer)indices, indices_alloc);
        }
    }

    void vk_mesh::set_data(const std::vector<nova_vertex>& vertex_data, const std::vector<int>& index_data) {
        this->data_format = data_format;

        upload_vertex_data(vertex_data);
        upload_index_data(index_data);
    }

    format vk_mesh::get_format() {
        return data_format;
    }

    bool vk_mesh::has_data() const {
        return num_indices > 0;
    }

    void vk_mesh::upload_vertex_data(const std::vector<nova_vertex> &vertex_data) {
        vk::BufferCreateInfo vertex_buffer_create = {};
        vertex_buffer_create.size = vertex_data.size() * sizeof(nova_vertex);
        vertex_buffer_create.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
        vertex_buffer_create.sharingMode = vk::SharingMode::eExclusive;
        vertex_buffer_create.queueFamilyIndexCount = 1;
        vertex_buffer_create.pQueueFamilyIndices = &context->graphics_family_idx;

        VmaAllocationCreateInfo vertex_buffer_alloc_create_info = {};
        vertex_buffer_alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        vmaCreateBuffer(context->allocator,
                        reinterpret_cast<VkBufferCreateInfo*>(&vertex_buffer_create), &vertex_buffer_alloc_create_info,
                        reinterpret_cast<VkBuffer*>(&vertex_buffer), &vertex_alloc, nullptr);

        void* mapped_vbo;
        vmaMapMemory(context->allocator, vertex_alloc, &mapped_vbo);
        memcpy(mapped_vbo, vertex_data.data(), vertex_buffer_create.size);
        vmaUnmapMemory(context->allocator, vertex_alloc);
    }

    void vk_mesh::upload_index_data(const std::vector<int>& index_data) {
        vk::BufferCreateInfo index_buffer_create = {};
        index_buffer_create.size = index_data.size() * sizeof(int);
        index_buffer_create.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
        index_buffer_create.sharingMode = vk::SharingMode::eExclusive;
        index_buffer_create.queueFamilyIndexCount = 1;
        index_buffer_create.pQueueFamilyIndices = &context->graphics_family_idx;

        VmaAllocationCreateInfo index_buffer_alloc_create_info = {};
        index_buffer_alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        vmaCreateBuffer(context->allocator,
                        reinterpret_cast<VkBufferCreateInfo*>(&index_buffer_create), &index_buffer_alloc_create_info,
                        reinterpret_cast<VkBuffer*>(&indices), &indices_alloc, nullptr);

        void* mapped_vbo;
        vmaMapMemory(context->allocator, indices_alloc, &mapped_vbo);
        memcpy(mapped_vbo, index_data.data(), index_buffer_create.size);
        vmaUnmapMemory(context->allocator, indices_alloc);

        num_indices = static_cast<uint32_t>(index_data.size());
    }
}
