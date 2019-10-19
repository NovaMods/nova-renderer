#include "default_new.hpp"

#include <stdexcept>

#if !defined(_WIN32) || defined(EA_PLATFORM_MINGW)
#	include <cstdlib>
#endif

namespace {
    void* std_aligned_alloc(const std::size_t size, const std::size_t alignment) {
        if (alignment == 0 || (alignment & (alignment - 1)) != 0)
            throw std::invalid_argument("Alignment must be a power of 2");

        void* allocatedMemory = nullptr;

        do {
#ifdef _WIN32
            allocatedMemory = _aligned_malloc(size, alignment);
#else
            alignment = alignment < alignof(void*) ? alignof(void*) : alignment;
            if (posix_memalign(&allocatedMemory, alignment, size) != 0) {
                allocatedMemory = nullptr;
            }
#endif

            if (!allocatedMemory) {
                const std::new_handler handler = std::get_new_handler();

                if (handler)
                    handler();
                else
                    throw std::bad_alloc();
            }
        } while (!allocatedMemory);

        return allocatedMemory;
    }

    void std_aligned_free(void* ptr) noexcept {
#ifdef _WIN32
        _aligned_free(ptr);
#else
        free(ptr);
#endif
    }
} // namespace

auto operator new(const std::size_t count) -> void* {
    return std_aligned_alloc(count, 16);
}

void* operator new[](const std::size_t count) {
    return ::operator new(count);
}

void* operator new(const std::size_t count, std::nothrow_t const& tag) noexcept {
    return malloc(count);
}

void* operator new[](const std::size_t count, std::nothrow_t const& tag) noexcept {
    return ::operator new(count, tag);
}

void* operator new[](const std::size_t size, char const* /*name*/, int /*flags*/, unsigned /*debugFlags*/, char const* /*file*/, int /*line*/) {
    return ::operator new(size);
}

void* operator new[](const std::size_t size,
                     const std::size_t alignment,
                     std::size_t /*alignmentOffset*/,
                     char const* /*name*/,
                     int /*flags*/,
                     unsigned /*debugFlags*/,
                     char const* /*file*/,
                     int /*line*/) {
    return std_aligned_alloc(size, alignment);
}

void operator delete(void* ptr) noexcept {
    std_aligned_free(ptr);
}

void operator delete[](void* ptr) noexcept {
    ::operator delete(ptr);
}

void operator delete(void* ptr, std::size_t count) noexcept {
    ::operator delete(ptr);
}

void operator delete[](void* ptr, std::size_t count) noexcept {
    ::operator delete(ptr, count);
}

void operator delete(void* ptr, std::nothrow_t const& tag) noexcept {
    ::operator delete(ptr);
}

void operator delete[](void* ptr, std::nothrow_t const& tag) noexcept {
    ::operator delete(ptr, tag);
}