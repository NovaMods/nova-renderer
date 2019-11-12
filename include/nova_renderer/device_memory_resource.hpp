#pragma once

#include "nova_renderer/polyalloc.hpp"
#include "nova_renderer/allocation_structs.hpp"
#include "nova_renderer/allocation_strategy.hpp"
#include "nova_renderer/bytes.hpp"

using namespace bvestl::polyalloc::operators;

namespace nova::renderer {
    namespace rhi {
        struct DeviceMemory;
    }

    struct DeviceMemoryAllocation {
        rhi::DeviceMemory* memory = nullptr;
        bvestl::polyalloc::AllocationInfo allocation_info;
    };

    /*!
     * \brief Couples an allocation strategy with a device memory object, allowing you to subdivide the device memory for use in individual
     * buffers, textures, etc
     */
    struct DeviceMemoryResource {
        DeviceMemoryResource(rhi::DeviceMemory* memory, bvestl::polyalloc::AllocationStrategy* allocation_strategy);

        [[nodiscard]] DeviceMemoryAllocation allocate(bvestl::polyalloc::Bytes size) const;

        bvestl::polyalloc::AllocationStrategy* allocation_strategy;

        rhi::DeviceMemory* memory;
    };
} // namespace nova::renderer
