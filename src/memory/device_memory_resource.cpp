#include "nova_renderer/device_memory_resource.hpp"

namespace nova::renderer {
    DeviceMemoryResource::DeviceMemoryResource(rhi::DeviceMemory* memory,
                                               foundational::allocation::SizeOnlyAllocator* allocation_strategy,
                                               bvestl::polyalloc::allocator_handle& host_allocator)
        : host_allocator(host_allocator), allocation_strategy(allocation_strategy), memory(memory) {}

    DeviceMemoryAllocation DeviceMemoryResource::allocate(const foundational::allocation::Bytes size) const {
        foundational::allocation::AllocationInfo alloc_info;
        allocation_strategy->allocate(size, alloc_info);

        return {memory, alloc_info};
    }
} // namespace nova::renderer