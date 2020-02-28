#pragma once

namespace nova {
    template <typename ValueType>
    class RaiiPtr {
    public:
        template <typename... Args>
        explicit RaiiPtr(rx::memory::allocator& allocator, Args... args);

        RaiiPtr(const RaiiPtr& other) = delete;
        RaiiPtr& operator=(const RaiiPtr& other) = delete;

        RaiiPtr(RaiiPtr&& old) noexcept;
        RaiiPtr& operator=(RaiiPtr&& old) noexcept;

        ~RaiiPtr();

        [[nodiscard]] ValueType* operator->();

        [[nodiscard]] const ValueType* operator->() const;

        [[nodiscard]] operator bool() const;

        [[nodiscard]] ValueType* operator&();

        [[nodiscard]] ValueType* operator&() const;

    private:
        rx::memory::allocator* allocator = nullptr;

        ValueType* val = nullptr;
    };

    template <typename ValueType>
    template <typename... Args>
    RaiiPtr<ValueType>::RaiiPtr(rx::memory::allocator& allocator, Args... args)
        : allocator{&allocator}, val{allocator.create<ValueType>(rx::utility::forward<Args>(args)...)} {}

    template <typename ValueType>
    RaiiPtr<ValueType>::RaiiPtr(RaiiPtr&& old) noexcept : allocator{old.allocator}, val{old.val} {
        old.val = nullptr;
    }

    template <typename ValueType>
    RaiiPtr<ValueType>& RaiiPtr<ValueType>::operator=(RaiiPtr&& old) noexcept {
        allocator = old.allocator;
        val = old.val;

        old.val = nullptr;

        return *this;
    }

    template <typename ValueType>
    RaiiPtr<ValueType>::~RaiiPtr() {
        if(val != nullptr) {
            allocator->destroy<ValueType>(val);
        }
    }

    template <typename ValueType>
    ValueType* RaiiPtr<ValueType>::operator->() {
        return val;
    }

    template <typename ValueType>
    const ValueType* RaiiPtr<ValueType>::operator->() const {
        return val;
    }

    template <typename ValueType>
    RaiiPtr<ValueType>::operator bool() const {
        return val != nullptr;
    }

    template <typename ValueType>
    ValueType* RaiiPtr<ValueType>::operator&() {
        return val;
    }

    template <typename ValueType>
    ValueType* RaiiPtr<ValueType>::operator&() const {
        return val;
    }
} // namespace nova
