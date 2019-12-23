#pragma once

#include <memory_resource>

namespace nova::memory {
    template <typename AllocatedType = std::byte>
    class AllocatorHandle : public std::pmr::polymorphic_allocator<AllocatedType> {
    public:
        explicit AllocatorHandle(std::pmr::memory_resource* memory) : std::pmr::polymorphic_allocator<std::byte>(memory) {}

        /*!
         * \brief Allocates and constructs an object of the specified type
         *
         * Would be nice if C++ came with this method, but nope not yet
         */
        template <typename... Args>
        AllocatedType* new_object(Args&&... args) {
            auto* mem = this->allocate(1);
            this->construct(mem, args...);
            return mem;
        }

        /*!
         * \brief Allocates an object of a different type than what this allocator normally created
         *
         * Intended use case is that you have a byte allocator that you allocate a few different object types from
         */
        template <typename ObjectType, typename = std::enable_if_t<std::is_same_v<AllocatedType, std::byte>>>
        ObjectType* allocate_object() {
            auto* mem = this->allocate(sizeof(ObjectType));
            return static_cast<ObjectType*>(mem);
        }

        /*!
         * \brief Allocates and constructs an object of a different type
         *
         * Intended use case is that you have a byte allocator that you use to create objects of different types
         */
        template <typename ObjectType, typename... Args, typename = std::enable_if_t<std::is_same_v<AllocatedType, std::byte>>>
        ObjectType* new_other_object(Args... args) {
            auto* mem = this->allocate(sizeof(ObjectType));
            return new(mem) ObjectType(std::forward<Args>(args)...);
        }

        template <typename ObjectType, typename = std::enable_if_t<std::is_same_v<AllocatedType, std::byte>>>
        AllocatorHandle<ObjectType>* create_suballocator() {
            return new_other_object<AllocatorHandle<ObjectType>>(this->resource());
        }
    };
} // namespace nova::memory
