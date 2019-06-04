/*!
 * \brief Vulkan definition of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois
 * \date 01-Apr-19.
 */

#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "nova_renderer/rhi_types.hpp"

namespace nova::renderer::rhi {

    struct VulkanMemoryAllocation : DeviceMemory {
        VmaAllocation allocation{};
        VmaAllocationInfo info{};
        VmaMemoryUsage usage = VMA_MEMORY_USAGE_GPU_ONLY;
        operator VmaAllocation&() { return allocation; };
        operator const VmaAllocation&() const { return allocation; };
        operator VmaAllocationInfo&() { return info; };
        operator const VmaAllocationInfo&() const { return info; };
    };

} // namespace nova::renderer::rhi
