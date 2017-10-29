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

    void vk_mesh::set_data(std::vector<int> vertex_data, format data_format, std::vector<int> index_data) {
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
        /*switch(data_format) {
            case format::POS:
                // We only need to set up positional data
                // Positions are always at vertex attribute 0
                glEnableVertexAttribArray(0);   // Position

                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

                break;

            case format::POS_UV:
                glEnableVertexAttribArray(0);   // Position
                glEnableVertexAttribArray(1);   // Texture UV

                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *) (3 * sizeof(GLfloat)));

                break;

            case format::POS_UV_COLOR:
                glEnableVertexAttribArray(0);   // Position
                glEnableVertexAttribArray(1);   // Texture UV
                glEnableVertexAttribArray(2);   // Vertex color

                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), nullptr);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void *) (3 * sizeof(GLfloat)));
                glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void *) (5 * sizeof(GLfloat)));

                break;

            case format::POS_COLOR_UV_LIGHTMAPUV_NORMAL_TANGENT:
                glEnableVertexAttribArray(0);   // Position
                glEnableVertexAttribArray(1);   // Texture UV
                glEnableVertexAttribArray(2);   // Lightmap UV
                glEnableVertexAttribArray(3);   // Normal
                glEnableVertexAttribArray(4);   // Tangent
                glEnableVertexAttribArray(5);   // Color

                // position
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), nullptr);

                // color
                glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_FALSE, 13 * sizeof(GLfloat), (void *) (12 * sizeof(GLbyte)));

                // texture UV
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void *) (16 * sizeof(GLbyte)));

                // lightmap UV
                glVertexAttribPointer(2, 2, GL_SHORT, GL_FALSE, 13 * sizeof(GLfloat), (void *) (24 * sizeof(GLbyte)));

                // normal
                glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void *) (32 * sizeof(GLbyte)));

                // tangent
                glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void *) (44 * sizeof(GLbyte)));

                break;
        }*/
    }

    format vk_mesh::get_format() {
        return data_format;
    }

    bool vk_mesh::has_data() const {
        return num_indices > 0;
    }

    void vk_mesh::upload_vertex_data(std::vector<int> &vertex_data, const render_context &context) {
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

    void vk_mesh::upload_index_data(std::vector<int> index_data, render_context &context) {
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
}
