#pragma once

#include <bvestl/polyalloc/polyalloc.hpp>
#include <foundational/allocation/bytes.hpp>
#include <foundational/allocation/allocation_structs.hpp>

using namespace foundational::allocation::operators;

namespace nova::renderer {

    struct DeviceMemoryAllocation {
        struct DeviceMemory* memory = nullptr;
        foundational::allocation::AllocationInfo allocation_info;
    };

    /*!
     * \brief Couples an allocation strategy with a device memory object, allowing you to subdivide the device memory for use in individual
     * buffers, textures, etc
     */
    template <typename AllocationStrategy>
    class DeviceMemoryResource {
    public:
        DeviceMemoryResource(struct DeviceMemory* memory,
                             bvestl::polyalloc::allocator_handle& host_allocator,
                             const foundational::allocation::Bytes size,
                             const foundational::allocation::Bytes alignment = 0_b)
            : allocation_strategy(host_allocator, size, alignment), memory(memory) {}

        DeviceMemoryAllocation allocate(foundational::allocation::Bytes size) { 
            const foundational::allocation::AllocationInfo alloc_info = allocation_strategy.allocate(size);

            return {memory, alloc_info};
        }

    private:
        AllocationStrategy allocation_strategy;

        struct DeviceMemory* memory;
    };
} // namespace nova::renderer
