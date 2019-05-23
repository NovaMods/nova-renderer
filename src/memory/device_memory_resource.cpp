#include "nova_renderer/device_memory_resource.hpp"

namespace nova::renderer {
    DeviceMemoryAllocation DeviceMemoryResource::allocate(const foundational::allocation::Bytes size) const {
        foundational::allocation::AllocationInfo alloc_info;
        allocation_strategy->allocate(size, alloc_info);

        return {memory, alloc_info};
    }
} // namespace nova::renderer