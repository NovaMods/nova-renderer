/*!
 * \author ddubois
 * \date 11-Nov-18.
 */

#include "aligned_block_allocator.hpp"
#include "../../util/logger.hpp"
#include "../../util/utils.hpp"
#include "../render_engine.hpp"
#include "vulkan_utils.hpp"

namespace nova {
    compacting_block_allocator::allocation_info* compacting_block_allocator::allocate(const uint32 size) {
        allocation_info* allocation;

        // First try to allocate from an existing pool
        for(block_allocator_buffer& buffer : m_pools) {
            allocation = buffer.allocate(size);
            if(allocation != nullptr) {
                return allocation;
            }
        }

        // If we couldn't allocate from a pool we'll need to create a new one.
        VkDeviceSize poolSize = hostVisible ? m_hostVisibleMemoryMB : m_deviceLocalMemoryMB;

        block_allocator_buffer* pool = new block_allocator_buffer(m_nextPoolId++, memoryTypeBits, poolSize, hostVisible);
        if(pool->Init()) {
            m_pools.Append(pool);
        } else {
            idLib::FatalError("compacting_block_allocator::Allocate: Could not allocate new memory pool.");
        }

        // Now that we've added the new pool, allocate directly from it.
        pool->allocate(size, align, allocation);

        return allocation;
    }

    /*
    =============
    compacting_block_allocator::AllocateFromPools
    =============
    */
    bool compacting_block_allocator::AllocateFromPools(const uint32 size, const uint32 align, const uint32 memoryTypeBits, const bool needHostVisible, allocation_info& allocation) {
        // Remember in Part 3 we got the memory properties when enumerating devices?  That comes in handy here. (vkcontext.gpu->memProps)
        const VkPhysicalDeviceMemoryProperties& physicalMemoryProperties = vkcontext.gpu->memProps;
        const int num = m_pools.Num();

        // If we need host visible memory set the appropriate bits.  Otherwise we assume device local.
        const VkMemoryPropertyFlags required = needHostVisible ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : 0;
        // Because it offers the best performance we prefer it, but don't require it.
        const VkMemoryPropertyFlags preferred = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        // Now iterate over the pools looking for a suitable one.
        // This is the first attempt of two.
        // 1.) We look for both required and preferred flags.
        // 2.) We look for just required flags.
        for(int i = 0; i < num; ++i) {
            block_allocator_buffer* pool = m_pools[i];
            const uint32 memoryTypeIndex = pool->memory_type_index;

            // If we need host visible and this pool doesn't support it continue
            if(needHostVisible && pool->host_visible == false) {
                continue;
            }

            // vkcontext.gpu->memProps enumerates the number of memory types supported.
            // When we create a pool we assign it a given index.  That's what we try to
            // match here.
            if(((memoryTypeBits >> memoryTypeIndex) & 1) == 0) {
                continue;
            }

            // Now try to match our required memory flags.
            const VkMemoryPropertyFlags properties = physicalMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;
            if((properties & required) != required) {
                continue;
            }

            // Now try to match our preferred memory flags.
            if((properties & preferred) != preferred) {
                continue;
            }

            // If this matches both required and preferred, go ahead and allocate from the pool.
            if(pool->allocate(size, align, allocation)) {
                return true;
            }
        }

        // On the second attempt we just look for required flags.  Otherwise it's the same as above.
        for(int i = 0; i < num; ++i) {
            block_allocator_buffer* pool = m_pools[i];
            const uint32 memoryTypeIndex = pool->memory_type_index;

            if(needHostVisible && pool->host_visible == false) {
                continue;
            }

            if(((memoryTypeBits >> memoryTypeIndex) & 1) == 0) {
                continue;
            }

            const VkMemoryPropertyFlags properties = physicalMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;
            if((properties & required) != required) {
                continue;
            }

            if(pool->allocate(size, align, allocation)) {
                return true;
            }
        }

        return false;
    }

    /*
    =============
    block_allocator_buffer::block_allocator_buffer
    =============
    */
    block_allocator_buffer::block_allocator_buffer(const uint32 id, const uint32 memoryTypeBits, const VkDeviceSize size, const bool hostVisible)
        : id(id), next_block_id(0), size(size), allocated(0), host_visible(hostVisible) {
        // Determine a suitable memory type index for this pool.
        // This is basically the same thing as compacting_block_allocator's AllocateFromPools
        memory_type_index = VK_FindMemoryTypeIndex(memoryTypeBits, hostVisible);
    }

    /*
    =============
    block_allocator_buffer::Init
    =============
    */
    bool block_allocator_buffer::Init() {
        if(memory_type_index == UINT64_MAX) {
            return false;
        }

        // Yes it really is this easy ( once you get here ).
        // Simply tell Vulkan the size and memory index you want to use.
        VkMemoryAllocateInfo memoryAllocateInfo = {};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = size;
        memoryAllocateInfo.memoryTypeIndex = memory_type_index;

        ID_VK_CHECK(vkAllocateMemory(vkcontext.device, &memoryAllocateInfo, NULL, &m_deviceMemory))

        if(m_deviceMemory == VK_NULL_HANDLE) {
            return false;
        }

        // If this is host visible we map it into our address space using m_data.
        if(host_visible) {
            ID_VK_CHECK(vkMapMemory(vkcontext.device, m_deviceMemory, 0, size, 0, (void**) &m_data));
        }

        // Setup the first block.
        head = new block_t();
        head->size = size;
        head->offset = 0;
        head->prev = NULL;
        head->next = NULL;
        head->free = true;

        return true;
    }

    /*
    =============
    block_allocator_buffer::Shutdown
    =============
    */
    void block_allocator_buffer::Shutdown() {
        // Unmap the memory
        if(host_visible) {
            vkUnmapMemory(vkcontext.device, m_deviceMemory);
        }

        // Free the memory
        vkFreeMemory(vkcontext.device, m_deviceMemory, NULL);

        block_t* prev = NULL;
        block_t* current = head;
        while(1) {
            if(current->next == NULL) {
                delete current;
                break;
            } else {
                prev = current;
                current = current->next;
                delete prev;
            }
        }

        head = NULL;
    }
}  // namespace nova