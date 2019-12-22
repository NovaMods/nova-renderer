#pragma once

#include "nova_renderer/memory/allocation_strategy.hpp"
#include "nova_renderer/memory/allocation_structs.hpp"
#include "nova_renderer/memory/bytes.hpp"

using namespace nova::memory::operators;

namespace nova::renderer {
    namespace rhi {
        struct DeviceMemory;
    }

    struct DeviceMemoryAllocation {
        rhi::DeviceMemory* memory = nullptr;
        memory::AllocationInfo allocation_info;
    };

    /*!
     * \brief Couples an allocation strategy with a device memory object, allowing you to subdivide the device memory for use in individual
     * buffers, textures, etc
     */
    struct DeviceMemoryResource {
        DeviceMemoryResource(rhi::DeviceMemory* memory, memory::AllocationStrategy* allocation_strategy);

        [[nodiscard]] DeviceMemoryAllocation allocate(memory::Bytes size) const;

        memory::AllocationStrategy* allocation_strategy;

        rhi::DeviceMemory* memory;
    };
} // namespace nova::renderer
