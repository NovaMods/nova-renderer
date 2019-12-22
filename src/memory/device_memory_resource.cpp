#include "nova_renderer/rhi/device_memory_resource.hpp"

namespace nova::renderer {
    DeviceMemoryResource::DeviceMemoryResource(rhi::DeviceMemory* memory, memory::AllocationStrategy* allocation_strategy)
        : allocation_strategy(allocation_strategy), memory(memory) {}

    DeviceMemoryAllocation DeviceMemoryResource::allocate(const memory::Bytes size) const {
        memory::AllocationInfo alloc_info;
        allocation_strategy->allocate(size, alloc_info);

        return {memory, alloc_info};
    }
} // namespace nova::renderer