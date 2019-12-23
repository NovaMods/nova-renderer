#include "nova_renderer/memory/mallocator.hpp"

#include <cstdlib>

namespace bvestl::polyalloc {
    void* Mallocator::allocate(const size_t n, int /* flags */) { return std::malloc(n); }

    void* Mallocator::allocate(const size_t n, size_t /* alignment */, size_t /* offset */, const int flags) {
        // TODO: Actually support aligned alloc
        return allocate(n, flags);
    }

    void Mallocator::deallocate(void* p, size_t /* n */) { std::free(p); }
} // namespace bvestl::polyalloc
