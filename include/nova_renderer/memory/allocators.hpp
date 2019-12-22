#pragma once

#include <memory_resource>

namespace nova::memory {
    template<typename AllocatedType = std::byte>
    using Allocator = std::pmr::polymorphic_allocator<AllocatedType>;
}
