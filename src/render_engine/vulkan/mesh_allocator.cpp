/*!
 * \author ddubois 
 * \date 11-Nov-18.
 */

#include "mesh_allocator.hpp"
#include "../../util/logger.hpp"

namespace nova {
    mesh_allocator::mesh_allocator(const VmaAllocator* alloc) : vma_alloc(alloc) {
        allocate_new_buffer();
    }

    void mesh_allocator::allocate_new_buffer() {
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = new_buffer_size;
        buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        mega_buffer new_buffer = {};

        const VkResult result = vmaCreateBuffer(*vma_alloc, &buffer_create_info, &allocation_create_info, &new_buffer.buffer, &new_buffer.allocation, &new_buffer.alloc_info);
        if(result == VK_SUCCESS) {
            // WHOOOOOO
            // Fill out the allocations
            for(uint32_t i = 0; i < new_buffer_size / buffer_part_size; i++) {
                buffer_range new_range = {};
                new_range.buffer = new_buffer.buffer;
                new_range.offset = i * new_buffer_size;
                new_buffer.available_ranges.push_back(new_range);
            }

            buffers.push_back(new_buffer);

        } else {
            // VMA couldn't create it. We're probably out of memory, which I don't want to handle right now
            // TODO: Handle out of memory
            NOVA_LOG(ERROR) << "Could not allocate a new VkBuffer. Error code " << result;
        }
    }
}
