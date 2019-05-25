#pragma once

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
    struct DeviceMemoryResource {
        DeviceMemoryResource(rhi::DeviceMemory* memory, foundational::allocation::SizeOnlyAllocator* allocation_strategy);

        [[nodiscard]] DeviceMemoryAllocation allocate(const foundational::allocation::Bytes size) const;

        foundational::allocation::SizeOnlyAllocator* allocation_strategy;

        rhi::DeviceMemory* memory;
    };
} // namespace nova::renderer
