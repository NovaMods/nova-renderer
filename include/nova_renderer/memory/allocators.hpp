#pragma once

#include <memory_resource>

namespace nova::memory {
    template <typename AllocatedType = std::byte>
    class AllocatorHandle : public std::pmr::polymorphic_allocator<AllocatedType> {
    public:
        // ReSharper disable once CppNonExplicitConvertingConstructor
        AllocatorHandle(std::pmr::memory_resource* memory) : std::pmr::polymorphic_allocator<AllocatedType>(memory) {}

        template <typename... Args>
        AllocatedType* new_object(Args&&... args) {
            auto* mem = this->allocate(1);
            return new(mem) AllocatedType(std::forward<Args>(args)...);
        }

        template <typename U, typename... Args, std::enable_if_t<std::is_same_v<AllocatedType, std::byte>, int> = 0>
        U* new_object(Args&&... args) {
            auto* mem = this->allocate(sizeof(U));
            return new(mem) U(std::forward<Args>(args)...);
        }

        /*!
         * \brief Creates a new allocator which allocates from the same memory pool, but allocates a different type of object
         */
        template <typename U, std::enable_if_t<std::is_same_v<AllocatedType, std::byte>, int> = 0>
        AllocatorHandle<U> specialize() {
            auto* new_allocator = new_object<AllocatorHandle<U>>(resource());
            return new_allocator;
        }
    };

    template <typename ValueType>
    using Vector = std::vector<ValueType, AllocatorHandle<ValueType>>;
} // namespace nova::memory
