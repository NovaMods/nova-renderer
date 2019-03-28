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

        vk_mesh mesh;
        mesh.num_vertices = input_mesh.vertex_data.size();
        mesh.num_indices = static_cast<uint32_t>(input_mesh.indices.size());

        vk_buffer vertex_data_staging_buffer;
        vk_buffer index_data_staging_buffer;

        {
            VkBufferCreateInfo vertex_create = {};
            vertex_create.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            vertex_create.size = vertex_size;
            vertex_create.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            vertex_create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo vertex_alloc_info = {};
            vertex_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            vmaCreateBuffer(vma_allocator, &vertex_create, &vertex_alloc_info, &mesh.vertex_buffer.buffer, &mesh.vertex_buffer.allocation, &mesh.vertex_buffer.alloc_info);
            
            vertex_create.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            vertex_alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            vertex_alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

            vmaCreateBuffer(vma_allocator, &vertex_create, &vertex_alloc_info, &vertex_data_staging_buffer.buffer, &vertex_data_staging_buffer.allocation, &vertex_data_staging_buffer.alloc_info);

            std::memcpy(vertex_data_staging_buffer.alloc_info.pMappedData, input_mesh.vertex_data.data(), vertex_size);
        }

        {
            VkBufferCreateInfo index_create = {};
            index_create.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            index_create.size = index_size;
            index_create.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            index_create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo index_alloc_info = {};
            index_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            vmaCreateBuffer(vma_allocator, &index_create, &index_alloc_info, &mesh.index_buffer.buffer, &mesh.index_buffer.allocation, &mesh.index_buffer.alloc_info);


            index_create.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            index_alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            index_alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

            vmaCreateBuffer(vma_allocator, &index_create, &index_alloc_info, &index_data_staging_buffer.buffer, &index_data_staging_buffer.allocation, &index_data_staging_buffer.alloc_info);

            std::memcpy(index_data_staging_buffer.alloc_info.pMappedData, input_mesh.indices.data(), index_size);
        }

        VkCommandPool pool = get_command_buffer_pool_for_current_thread(transfer_family_index);

        VkCommandBufferAllocateInfo cmd_alloc = {};
        cmd_alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_alloc.commandPool = pool;
        cmd_alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmd_alloc.commandBufferCount = 1;

        VkCommandBuffer cmds;
        vkAllocateCommandBuffers(device, &cmd_alloc, &cmds);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmds, &begin_info);

        VkBufferCopy vertex_copy = {};
        vertex_copy.size = vertex_size;
        vkCmdCopyBuffer(cmds, vertex_data_staging_buffer.buffer, mesh.vertex_buffer.buffer, 1, &vertex_copy);

        VkBufferCopy index_copy = {};
        index_copy.size = index_size;
        vkCmdCopyBuffer(cmds, index_data_staging_buffer.buffer, mesh.index_buffer.buffer, 1, &index_copy);

        vkEndCommandBuffer(cmds);

        VkFenceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        VkFence copy_done_fence;
        vkCreateFence(device, &create_info, nullptr, &copy_done_fence);

        submit_to_queue(cmds, copy_queue, copy_done_fence);

        vkWaitForFences(device, 1, &copy_done_fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

        vkDestroyFence(device, copy_done_fence, nullptr);
        vmaDestroyBuffer(vma_allocator, vertex_data_staging_buffer.buffer, vertex_data_staging_buffer.allocation);
        vmaDestroyBuffer(vma_allocator, index_data_staging_buffer.buffer, index_data_staging_buffer.allocation);

        vkFreeCommandBuffers(device, pool, 1, &cmds);

        mesh.id = next_mesh_id;
        next_mesh_id.fetch_add(1);

        return result<mesh_id_t>(mesh.id);
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

        NOVA_CHECK_RESULT(vmaCreateBuffer(vma_allocator,
                                               &buffer_create_info,
                                               &allocation_create_info,
                                               &new_buffer.buffer,
                                               &new_buffer.allocation,
                                               &new_buffer.alloc_info));

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

        model_matrix_buffer->record_buffer_upload(ubo_uploads);

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

        NOVA_CHECK_RESULT(vkEndCommandBuffer(ubo_uploads));

        VkFence dummy_fence = model_matrix_buffer->get_dummy_fence();
        (void) dummy_fence;
        // NOVA_CHECK_RESULT(vkWaitForFences(device, 1, &dummy_fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        // NOVA_CHECK_RESULT(vkResetFences(device, 1, &dummy_fence));

        submit_to_queue(ubo_uploads, copy_queue);
    }

    void vulkan_render_engine::delete_mesh(uint32_t mesh_id) {
        const vk_mesh mesh = meshes.at(mesh_id);
        meshes.erase(mesh_id);

        vmaDestroyBuffer(vma_allocator, mesh.index_buffer.buffer, mesh.index_buffer.allocation);
        vmaDestroyBuffer(vma_allocator, mesh.vertex_buffer.buffer, mesh.vertex_buffer.allocation);
    }
} // namespace nova::renderer
