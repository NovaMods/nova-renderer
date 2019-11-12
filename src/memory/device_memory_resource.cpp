#include "nova_renderer/device_memory_resource.hpp"

namespace nova::renderer {
    DeviceMemoryResource::DeviceMemoryResource(rhi::DeviceMemory* memory, bvestl::polyalloc::AllocationStrategy* allocation_strategy)
        : allocation_strategy(allocation_strategy), memory(memory) {}

    DeviceMemoryAllocation DeviceMemoryResource::allocate(const bvestl::polyalloc::Bytes size) const {
        bvestl::polyalloc::AllocationInfo alloc_info;
        allocation_strategy->allocate(size, alloc_info);

        return {memory, alloc_info};
    }
} // namespace nova::renderer