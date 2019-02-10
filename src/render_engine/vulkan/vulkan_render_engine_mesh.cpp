/*!
 * \author ddubois
 * \date 07-Feb-19.
 */

#include "vulkan_render_engine.hpp"
#include "vulkan_utils.hpp"

namespace nova {
    mesh_id_t vulkan_render_engine::add_mesh(const mesh_data &input_mesh) {
        const auto vertex_size = static_cast<uint32_t>(input_mesh.vertex_data.size() * sizeof(full_vertex));
        const auto index_size = static_cast<uint32_t>(input_mesh.indices.size() * sizeof(uint32_t));

        // TODO: Make the extra memory allocation configurable
        const auto total_memory_needed = static_cast<uint32_t>(std::round((vertex_size + index_size) * 1.1)); // Extra size so chunks can grow

        const vk_buffer staging_buffer = get_or_allocate_mesh_staging_buffer(total_memory_needed);
        std::memcpy(staging_buffer.alloc_info.pMappedData, &input_mesh.vertex_data[0], vertex_size);
        std::memcpy(reinterpret_cast<uint8_t *>(staging_buffer.alloc_info.pMappedData) + vertex_size, &input_mesh.indices[0], index_size);

        const uint32_t mesh_id = next_mesh_id.fetch_add(1);

        std::lock_guard l(mesh_upload_queue_mutex);
        mesh_upload_queue.push(mesh_staging_buffer_upload_command{staging_buffer, mesh_id, vertex_size, vertex_size + index_size});

        return mesh_id;
    }

    vk_buffer vulkan_render_engine::get_or_allocate_mesh_staging_buffer(const uint32_t needed_size) {
        std::lock_guard l(mesh_staging_buffers_mutex);

        if(!available_mesh_staging_buffers.empty()) {
            // Try to find a buffer that's big enough
            uint32_t potential_staging_buffer_idx = std::numeric_limits<uint32_t>::max();

            for(size_t i = 0; i < available_mesh_staging_buffers.size(); i++) {
                if(available_mesh_staging_buffers[i].alloc_info.size >= needed_size && available_mesh_staging_buffers[i].alloc_info.size > available_mesh_staging_buffers[potential_staging_buffer_idx].alloc_info.size) {
                    potential_staging_buffer_idx = static_cast<uint32_t>(i);
                }
            }

            if(potential_staging_buffer_idx < available_mesh_staging_buffers.size()) {
                const vk_buffer staging_buffer = available_mesh_staging_buffers[potential_staging_buffer_idx];
                available_mesh_staging_buffers.erase(available_mesh_staging_buffers.begin() + potential_staging_buffer_idx);
                return staging_buffer;
            }
        }

        vk_buffer new_buffer = {};

        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_create_info.size = needed_size;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &copy_family_index;

        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        allocation_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        NOVA_THROW_IF_VK_ERROR(vmaCreateBuffer(vma_allocator, &buffer_create_info, &allocation_create_info, &new_buffer.buffer, &new_buffer.allocation, &new_buffer.alloc_info), render_engine_rendering_exception);

        return new_buffer;
    }

    void vulkan_render_engine::delete_mesh(uint32_t mesh_id) {
        const vk_mesh mesh = meshes.at(mesh_id);
        meshes.erase(mesh_id);

        mesh_memory->free(mesh.memory);
    }
} // namespace nova
