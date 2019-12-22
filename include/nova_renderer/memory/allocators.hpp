#pragma once

#include <memory_resource>

namespace nova::memory {
    template <typename AllocatedType = std::byte>
    class Allocator : public std::pmr::polymorphic_allocator<AllocatedType> {
    public:
        // ReSharper disable once CppNonExplicitConvertingConstructor
        Allocator(std::pmr::memory_resource* memory) : std::pmr::polymorphic_allocator(memory) {}

        template <typename... Args>
        AllocatedType* new_object(Args&&... args) {
            auto* mem = this->allocate(1);
            return new(mem) AllocatedType(std::forward<Args>(args)...);
        }

        template <typename U,
                  typename... Args,
                  typename = typename std::enable_if<std::is_same<AllocatedType, std::byte>::value>::type>
        U* new_object(Args&&... args) {
            auto* mem = this->allocate(sizeof(U));
            return new(mem) U(std::forward<Args>(args)...);
        }
    };
} // namespace nova::memory
