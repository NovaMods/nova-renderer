#pragma once

#include "nova_renderer/polyalloc.hpp"

namespace bvestl::polyalloc {
    /*!
     * \brief An allocator which uses `malloc` to allocate all memory
     *
     * Intended use case is bootstrapping everything - at some point you have to allocate memory from the heap
     */
    class Mallocator final : public Allocator {
        void* allocate(size_t n, int flags) override;

        void* allocate(size_t n, size_t alignment, size_t offset, int flags) override;

        void deallocate(void* p, size_t n) override;
    };
} // namespace bvestl::polyalloc
