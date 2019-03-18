/*!
 * \author ddubois
 * \date 07-Feb-19.
 */

#include "vulkan_render_engine.hpp"
#include "vulkan_utils.hpp"

namespace nova::renderer {
    result<mesh_id_t> vulkan_render_engine::add_mesh(const mesh_data& input_mesh) {
        const auto vertex_size = static_cast<uint32_t>(input_mesh.vertex_data.size() * sizeof(full_vertex));
        const auto index_size = static_cast<uint32_t>(input_mesh.indices.size() * sizeof(uint32_t));

        // TODO: Make the extra memory allocation configurable
        const auto total_memory_needed = static_cast<uint32_t>(
            std::round((vertex_size + index_size) * 1.1)); // Extra size so chunks can grow

        const vk_buffer staging_buffer = get_or_allocate_mesh_staging_buffer(total_memory_needed);
        std::memcpy(staging_buffer.alloc_info.pMappedData, &input_mesh.vertex_data[0], vertex_size);
        std::memcpy(reinterpret_cast<uint8_t*>(staging_buffer.alloc_info.pMappedData) + vertex_size, &input_mesh.indices[0], index_size);

        uint32_t mesh_id = next_mesh_id.fetch_add(1);

        std::lock_guard l(mesh_upload_queue_mutex);
        mesh_upload_queue.push(mesh_staging_buffer_upload_command{staging_buffer, mesh_id, vertex_size, vertex_size + index_size});

        return result<mesh_id_t>(std::move(mesh_id));
    }

    vk_buffer vulkan_render_engine::get_or_allocate_mesh_staging_buffer(const uint32_t needed_size) {
        std::lock_guard l(mesh_staging_buffers_mutex);

        if(!available_mesh_staging_buffers.empty()) {
            // Try to find a buffer that's big enough
            uint32_t potential_staging_buffer_idx = std::numeric_limits<uint32_t>::max();

            for(size_t i = 0; i < available_mesh_staging_buffers.size(); i++) {
                if(available_mesh_staging_buffers[i].alloc_info.size >= needed_size &&
                   available_mesh_staging_buffers[i].alloc_info.size >
                       available_mesh_staging_buffers[potential_staging_buffer_idx].alloc_info.size) {
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
        buffer_create_info.pQueueFamilyIndices = &transfer_family_index;

        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        allocation_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        NOVA_THROW_IF_VK_ERROR(vmaCreateBuffer(vma_allocator,
                                               &buffer_create_info,
                                               &allocation_create_info,
                                               &new_buffer.buffer,
                                               &new_buffer.allocation,
                                               &new_buffer.alloc_info),
                               render_engine_rendering_exception);

        return new_buffer;
    }

    void vulkan_render_engine::upload_new_ubos() {
        VkCommandPool ubo_upload_pool = get_command_buffer_pool_for_current_thread(transfer_family_index);

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = ubo_upload_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer ubo_uploads;
        vkAllocateCommandBuffers(device, &alloc_info, &ubo_uploads);

        VkCommandBufferBeginInfo begin = {};
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(ubo_uploads, &begin);

        VkBufferMemoryBarrier static_mesh_ubo_barrier_start = {};
        static_mesh_ubo_barrier_start.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        static_mesh_ubo_barrier_start.srcAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
        static_mesh_ubo_barrier_start.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        static_mesh_ubo_barrier_start.srcQueueFamilyIndex = graphics_family_index;
        static_mesh_ubo_barrier_start.dstQueueFamilyIndex = transfer_family_index;
        static_mesh_ubo_barrier_start.buffer = model_matrix_buffer->get_vk_buffer();
        static_mesh_ubo_barrier_start.size = model_matrix_buffer->get_size();

        // TODO: Other barriers to send other UBOs to the transfer queue

        vkCmdPipelineBarrier(ubo_uploads,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT,
                             0,
                             nullptr,
                             1,
                             &static_mesh_ubo_barrier_start,
                             0,
                             nullptr);

        model_matrix_buffer->record_ubo_upload(ubo_uploads);

        VkBufferMemoryBarrier static_mesh_ubo_barrier_end = static_mesh_ubo_barrier_start;
        static_mesh_ubo_barrier_end.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        static_mesh_ubo_barrier_end.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
        static_mesh_ubo_barrier_end.srcQueueFamilyIndex = transfer_family_index;
        static_mesh_ubo_barrier_end.dstQueueFamilyIndex = graphics_family_index;

        vkCmdPipelineBarrier(ubo_uploads,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT,
                             0,
                             nullptr,
                             1,
                             &static_mesh_ubo_barrier_end,
                             0,
                             nullptr);

        vkEndCommandBuffer(ubo_uploads);

        VkFence dummy_fence = model_matrix_buffer->get_dummy_fence();
        vkResetFences(device, 1, &dummy_fence);

        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &ubo_uploads;
        vkQueueSubmit(copy_queue, 1, &submit, model_matrix_buffer->get_dummy_fence());
    }

    void vulkan_render_engine::delete_mesh(uint32_t mesh_id) {
        const vk_mesh mesh = meshes.at(mesh_id);
        meshes.erase(mesh_id);

        mesh_memory->free(mesh.memory);
    }
} // namespace nova::renderer
