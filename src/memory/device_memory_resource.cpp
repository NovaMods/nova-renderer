#include "nova_renderer/rhi/device_memory_resource.hpp"

namespace nova::renderer {
    DeviceMemoryResource::DeviceMemoryResource(rhi::RhiDeviceMemory* memory, mem::AllocationStrategy* allocation_strategy)
        : allocation_strategy(allocation_strategy), memory(memory) {}

    DeviceMemoryAllocation DeviceMemoryResource::allocate(const mem::Bytes size) const {
        mem::AllocationInfo alloc_info;
        allocation_strategy->allocate(size, alloc_info);

        return {memory, alloc_info};
    }
} // namespace nova::renderer