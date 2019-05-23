#pragma once

#include <bvestl/polyalloc/polyalloc.hpp>
#include <foundational/allocation/allocation_structs.hpp>
#include <foundational/allocation/bytes.hpp>
#include <foundational/allocation/size_only_allocator.hpp>

using namespace foundational::allocation::operators;

namespace nova::renderer {
    namespace rhi {
        struct DeviceMemory;
    }

    struct DeviceMemoryAllocation {
        rhi::DeviceMemory* memory = nullptr;
        foundational::allocation::AllocationInfo allocation_info;
    };

    /*!
     * \brief Couples an allocation strategy with a device memory object, allowing you to subdivide the device memory for use in individual
     * buffers, textures, etc
     */
    class DeviceMemoryResource {
    public:
        DeviceMemoryResource(rhi::DeviceMemory* memory,
                             foundational::allocation::SizeOnlyAllocator* allocation_strategy,
                             bvestl::polyalloc::allocator_handle& host_allocator);

        [[nodiscard]] DeviceMemoryAllocation allocate(const foundational::allocation::Bytes size) const;

    private:
        bvestl::polyalloc::allocator_handle& host_allocator;

        foundational::allocation::SizeOnlyAllocator* allocation_strategy;

        struct DeviceMemory* memory;
    };
} // namespace nova::renderer
