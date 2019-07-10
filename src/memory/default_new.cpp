#include "default_new.hpp"

#include <EABase/eabase.h>

#include <stdexcept>

#if !defined(EA_PLATFORM_MICROSOFT) || defined(EA_PLATFORM_MINGW)
#	include <cstdlib>
#endif

namespace {
    void* eastl_aligned_alloc(std::size_t size, std::size_t alignment) {
        if (alignment == 0 ||  (alignment & (alignment - 1)) == 0)
            throw std::invalid_argument("Alignment must be a power of 2");

        void* allocatedMemory = nullptr;

        do {
#ifdef EA_PLATFORM_MICROSOFT
            allocatedMemory = _aligned_malloc(size, alignment);
#else
            alignment = alignment < alignof(void*) ? alignof(void*) : alignment;
            if (posix_memalign(&allocatedMemory, alignment, size) != 0) {
                allocatedMemory = nullptr;
            }
#endif

            if (!allocatedMemory) {
                std::new_handler handler = std::get_new_handler();

                if (handler)
                    handler();
                else
                    throw std::bad_alloc();
            }
        } while (!allocatedMemory);

        return allocatedMemory;
    }

    void eastl_aligned_free(void* ptr) noexcept {
#ifdef EA_PLATFORM_MICROSOFT
        _aligned_free(ptr);
#else
        free(ptr);
#endif
    }
} // namespace

void* operator new(std::size_t count) {
    return eastl_aligned_alloc(count, 16);
}

void* operator new[](std::size_t count) {
    return ::operator new(count);
}

void* operator new(std::size_t count, std::nothrow_t const& tag) noexcept {
    EA_UNUSED(tag);

    return malloc(count);
}

void* operator new[](std::size_t count, std::nothrow_t const& tag) noexcept {
    return ::operator new(count, tag);
}

void* operator new[](std::size_t size, char const* /*name*/, int /*flags*/, unsigned /*debugFlags*/, char const* /*file*/, int /*line*/) {
    return ::operator new(size);
}

void* operator new[](std::size_t size,
                     std::size_t alignment,
                     std::size_t /*alignmentOffset*/,
                     char const* /*name*/,
                     int /*flags*/,
                     unsigned /*debugFlags*/,
                     char const* /*file*/,
                     int /*line*/) {
    return eastl_aligned_alloc(size, alignment);
}

void operator delete(void* ptr) noexcept {
    eastl_aligned_free(ptr);
}

void operator delete[](void* ptr) noexcept {
    ::operator delete(ptr);
}

void operator delete(void* ptr, std::size_t count) noexcept {
    EA_UNUSED(count);
    ::operator delete(ptr);
}

void operator delete[](void* ptr, std::size_t count) noexcept {
    ::operator delete(ptr, count);
}

void operator delete(void* ptr, std::nothrow_t const& tag) noexcept {
    EA_UNUSED(tag);
    ::operator delete(ptr);
}

void operator delete[](void* ptr, std::nothrow_t const& tag) noexcept {
    ::operator delete(ptr, tag);
}