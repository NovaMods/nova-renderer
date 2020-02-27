#pragma once

namespace nova {
    template<typename ValueType>
    class RaiiPtr {
    public:
        explicit RaiiPtr(rx::memory::allocator* allocator);
    };
}
