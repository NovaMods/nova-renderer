#include "mallocator.hpp"

namespace bvestl {
    namespace polyalloc {
        void* Mallocator::allocate(const size_t n, int /* flags */) {
			return std::malloc(n);
        }

        void* Mallocator::allocate(const size_t n, size_t /* alignment */, size_t /* offset */, const int flags) {
			return allocate(n, flags);
        }

        void Mallocator::deallocate(void* p, size_t /* n */) {
			std::free(p);
        }
    }
}