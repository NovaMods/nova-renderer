#pragma once

#include <bvestl/polyalloc/polyalloc.hpp>
#include <foundational/allocation/bytes.hpp>
#include <foundational/allocation/allocation_structs.hpp>
#include <foundational/allocation/size_only_allocator.hpp>

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
    class DeviceMemoryResource {
    public:
        template <typename AllocationStrategy>
        DeviceMemoryResource(struct DeviceMemory* memory,
                             bvestl::polyalloc::allocator_handle& host_allocator,
                             const foundational::allocation::Bytes size,
                             const foundational::allocation::Bytes alignment = 0_b)
            : host_allocator(host_allocator), memory(memory) {
            void* alloc_strat = host_allocator.allocate(sizeof(AllocationStrategy));
            allocation_strategy = new(alloc_strat) AllocationStrategy(host_allocator, size, alignment);
        }

        [[nodiscard]] DeviceMemoryAllocation allocate(const foundational::allocation::Bytes size) const;

    private:
        bvestl::polyalloc::allocator_handle& host_allocator;

        foundational::allocation::SizeOnlyAllocator* allocation_strategy;

        struct DeviceMemory* memory;
    };
} // namespace nova::renderer
