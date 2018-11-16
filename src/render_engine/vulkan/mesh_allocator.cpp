/*!
 * \author ddubois 
 * \date 11-Nov-18.
 */

#include "mesh_allocator.hpp"
#include "../../util/logger.hpp"
#include "../../util/utils.hpp"
#include "vulkan_utils.hpp"

namespace nova {
    mesh_allocator::mesh_allocator(const uint64_t max_size, const VmaAllocator* alloc, ftl::TaskScheduler* task_scheduler, uint32_t graphics_queue_idx, uint32_t copy_queue_idx)
        : vma_alloc(alloc), max_size(max_size), buffer_fibtex(task_scheduler), graphics_queue_idx(graphics_queue_idx), copy_queue_idx(copy_queue_idx) {

        allocate_new_buffer();
    }

    mesh_allocator::~mesh_allocator() {
        for(const auto& [buf, buf_info] : buffers) {
            vmaDestroyBuffer(*vma_alloc, buf, buf_info.allocation);
        }
    }

    buffer_range mesh_allocator::get_buffer_part() {
        ftl::LockGuard<ftl::Fibtex> buffer_guard(buffer_fibtex);
        for(auto& buf : buffers) {
            if(!buf.second.available_ranges.empty()) {
                // There's space. WHOOOOOO
                const buffer_range ret_val = buf.second.available_ranges.back();
                buf.second.available_ranges.pop_back();
                return ret_val;
            }
        }

        // Didn't find a buffer with space? Guess we'll make a new one....
        auto [buffer, buffer_info] = allocate_new_buffer();

        const buffer_range ret_val = buffer_info.available_ranges.back();
        buffer_info.available_ranges.pop_back();
        return ret_val;
    }

    mesh_memory mesh_allocator::allocate_mesh(const uint64_t size) {
        mesh_memory new_memory;
        new_memory.allocated_size = size;
        uint64_t size_remaining = size;
        while(size_remaining > buffer_part_size) {
            const buffer_range range = get_buffer_part();
            new_memory.parts.push_back(range);

            if(size_remaining < buffer_part_size) {
                break;
            }
            size_remaining -= buffer_part_size;
        }

        return new_memory;
    }

    void mesh_allocator::free_mesh(const mesh_memory& memory_to_free) {
        for(const buffer_range& part : memory_to_free.parts) {
            ftl::LockGuard<ftl::Fibtex> buffer_guard(buffer_fibtex);
            buffers.at(part.buffer).available_ranges.push_back(part);
        }
    }

    uint64_t mesh_allocator::get_num_bytes_allocated() const {
        return buffers.size() * new_buffer_size;
    }

    uint64_t mesh_allocator::get_num_bytes_used() const {
        const uint64_t num_buffer_parts = new_buffer_size / buffer_part_size;
        uint64_t num_buffer_parts_used = 0;
        for(const auto& [buf, buf_info] : buffers) {
            num_buffer_parts_used += num_buffer_parts - buf_info.available_ranges.size();
        }
        return num_buffer_parts_used * buffer_part_size;
    }

    uint64_t mesh_allocator::get_num_bytes_available() const {
        uint64_t num_buffer_parts_available = 0;
        for(const auto& [buf, buf_info] : buffers) {
            num_buffer_parts_available += buf_info.available_ranges.size();
        }

        return num_buffer_parts_available * buffer_part_size;
    }
    
    void mesh_allocator::add_barriers_before_mesh_upload(VkCommandBuffer cmds) {
        std::vector<VkBufferMemoryBarrier> barriers;
        barriers.reserve(buffers.size());
        for(const auto& [buf, buf_info] : buffers) {
            VkBufferMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.srcQueueFamilyIndex = graphics_queue_idx;
            barrier.dstQueueFamilyIndex = copy_queue_idx;
            barrier.buffer = buf;
            barrier.offset = 0;
            barrier.size = new_buffer_size;

            barriers.push_back(barrier);
        }

        vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, buffers.size(), barriers.data(), 0, nullptr);
    }

    void mesh_allocator::add_barriers_after_mesh_upload(VkCommandBuffer cmds) {
        std::vector<VkBufferMemoryBarrier> barriers;
        barriers.reserve(buffers.size());
        for(const auto& [buf, buf_info] : buffers) {
            VkBufferMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
            barrier.srcQueueFamilyIndex = copy_queue_idx;
            barrier.dstQueueFamilyIndex = graphics_queue_idx;
            barrier.buffer = buf;
            barrier.offset = 0;
            barrier.size = new_buffer_size;

            barriers.push_back(barrier);
        }

        vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, buffers.size(), barriers.data(), 0, nullptr);
    }

    std::pair<VkBuffer, mesh_allocator::mega_buffer_info&> mesh_allocator::allocate_new_buffer() {
        ftl::LockGuard<ftl::Fibtex> buffer_guard(buffer_fibtex);
        if(get_num_bytes_used() + new_buffer_size > max_size) {
            throw out_of_gpu_memory("Cannot exceed max size of " + std::to_string(max_size));
        }

        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = new_buffer_size;
        buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkBuffer buffer;
        mega_buffer_info new_buffer = {};

        const VkResult result = vmaCreateBuffer(*vma_alloc, &buffer_create_info, &allocation_create_info, &buffer, &new_buffer.allocation, &new_buffer.alloc_info);
        if(result == VK_SUCCESS) {
            // WHOOOOOO
            // Fill out the allocations
            for(uint32_t i = 0; i < new_buffer_size / buffer_part_size; i++) {
                buffer_range new_range = {};
                new_range.buffer = buffer;
                new_range.offset = i * new_buffer_size;
                new_buffer.available_ranges.push_back(new_range);
            }

            buffers.emplace(buffer, new_buffer);
            return { buffer, new_buffer };

        } else {
            // VMA couldn't create it. We're probably out of memory, which I don't want to handle right now
            // TODO: Handle out of memory
            NOVA_LOG(ERROR) << "Could not allocate a new VkBuffer. Error code " << vulkan::vulkan_utils::vk_result_to_string(result);
            throw out_of_gpu_memory("Could not allocated another mesh data buffer");
        }
    }
}
