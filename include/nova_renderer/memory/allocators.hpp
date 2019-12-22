#pragma once

#include <memory_resource>

namespace nova {
    template<typename AllocatedType = std::byte>
    using Allocator = std::pmr::polymorphic_allocator<AllocatedType>;
}
