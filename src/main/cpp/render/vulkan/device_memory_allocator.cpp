/*!
 * \author ddubois 
 * \date 17-Oct-17.
 */

#include <easylogging++.h>
#include "device_memory_allocator.h"
#include "render_device.h"

namespace nova {
    device_memory_allocator device_memory_allocator::instance;

    device_memory_allocator &device_memory_allocator::get_instance() {
        return instance;
    }

    allocation
    device_memory_allocator::allocate(const uint32_t size, const uint32_t align, const uint32_t memoty_type_bits,
                                      const bool host_visible) {
        allocation alloc;
        if(allocate_from_pools(size, align, memoty_type_bits, host_visible, alloc)) {
            return alloc;
        }

        // We couldn't allocate the memory from our pools, so let's make a new pool
        vk::DeviceSize pool_size = host_visible ? host_visible_memory_mb : device_local_memory_mb;

        auto* pool = new memory_pool(next_pool_id++, memoty_type_bits, pool_size, host_visible);
        if(pool->init()) {
            pools.push_back(pool);
        } else {
            LOG(FATAL) << "device_memory_allocator::allocate: Could not allocate pool and we're kinda out of memory";
        }

        pool->allocate(size, align, alloc);

        return alloc;
    }

    bool device_memory_allocator::allocate_from_pools(const uint32_t size, const uint32_t align,
                                                      const uint32_t memory_type_bits, const bool need_host_visible,
                                                      allocation &alloc) {
        const vk::PhysicalDeviceMemoryProperties& physical_memory_properties = render_device::instance.gpu.mem_props;
        std::size_t num = pools.size();

        auto required_flags = need_host_visible ? vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent : vk::MemoryPropertyFlagBits(0);

        const vk::MemoryPropertyFlags preferred_flags = vk::MemoryPropertyFlagBits::eDeviceLocal;

        // Look for a pool with the required and the preferred flags
        for(memory_pool* pool : pools) {
            const uint32_t memory_type_index = pool->memory_type_index;

            if(need_host_visible && !pool->host_visible) {
                continue;
            }

            // I am graphics and so can you tells me this is important
            if(((memory_type_bits >> memory_type_index) & 1) == 0) {
                continue;
            }

            const vk::MemoryPropertyFlags properties = physical_memory_properties.memoryTypes[memory_type_index].propertyFlags;
            if((properties & required_flags) != required_flags) {
                // Don't have the required flags? Try something else
                continue;
            }

            if((properties & preferred_flags) != preferred_flags) {
                // Don't have the preferred flags? This is just the first pass through the pools so we can safely ignore it
                continue;
            }

            if(pool->allocate(size, align, alloc)) {
                return true;
            }
        }

        // Next pass - only check for required flags
        for(memory_pool* pool : pools) {
            const uint32_t memory_type_index = pool->memory_type_index;

            if(need_host_visible && !pool->host_visible) {
                continue;
            }

            if(((memory_type_bits >> memory_type_index) & 1) == 0) {
                continue;
            }

            const vk::MemoryPropertyFlags properties = physical_memory_properties.memoryTypes[memory_type_index].propertyFlags;
            if((properties & required_flags) != required_flags) {
                // Don't have the required flags? Try something else
                continue;
            }

            if(pool->allocate(size, align, alloc)) {
                return true;
            }
        }

        // Couldn't allocate. That sucks but it's life
        return false;
    }

    memory_pool::memory_pool(const uint32_t id, const uint32_t memory_type_bits, const vk::DeviceSize size, const bool is_host_visible)
            : size(size), host_visible(is_host_visible), id(id), next_block_id(0), allocated(0) {

        const vk::PhysicalDeviceMemoryProperties& physical_memory_properties = render_device::instance.gpu.mem_props;

        const vk::MemoryPropertyFlags required_flags = host_visible ? vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent : vk::MemoryPropertyFlagBits(0);

        const vk::MemoryPropertyFlags preferred_flags = vk::MemoryPropertyFlagBits::eDeviceLocal;

        // Look for a pool with the required and the preferred flags
        for(uint32_t i = 0; i < 32; i++) {
            const vk::MemoryPropertyFlags properties = physical_memory_properties.memoryTypes[i].propertyFlags;
            if((properties & required_flags) != required_flags) {
                // Don't have the required flags? Try something else
                continue;
            }

            if((properties & preferred_flags) != preferred_flags) {
                // Don't have the preferred flags? This is just the first pass through the pools so we can safely ignore it
                continue;
            }

            if((properties & vk::MemoryPropertyFlagBits(memory_type_bits)) != vk::MemoryPropertyFlagBits(memory_type_bits)) {
                continue;
            }

            memory_type_index = i;
            break;
        }
    }
}
