#ifndef RX_CORE_OPTIONAL_H
#define RX_CORE_OPTIONAL_H
#include "rx/core/assert.h"                       // RX_ASSERT
#include "rx/core/memory/uninitialized_storage.h" // uninitialized_storage
#include "rx/core/utility/move.h"

namespace rx {

    namespace detail {
        struct nullopt {};
    }; // namespace detail

    constexpr detail::nullopt nullopt;

    template <typename T>
    struct optional {
        constexpr optional(detail::nullopt);
        constexpr optional();
        constexpr optional(T&& data_);
        constexpr optional(const T& _data);
        constexpr optional(optional&& other_);
        constexpr optional(const optional& _other);

        ~optional();

        optional& operator=(T&& data_);
        optional& operator=(const T& _data);
        optional& operator=(optional&& other_);
        optional& operator=(const optional& _other);

        operator bool() const;

        T& operator*();
        const T& operator*() const;
        T* operator->();
        const T* operator->() const;

        const T& value_or(const T& val) const;

        template <typename OtherType, typename FuncType>
        optional<OtherType> map(FuncType&& map_func);

        template <typename OtherType, typename FuncType>
        optional<OtherType> flat_map(FuncType&& map_func);

    private:
        memory::uninitialized_storage<T> m_data;
        bool m_init;
    };

    template <typename T>
    inline constexpr optional<T>::optional(decltype(nullopt)) : m_data{}, m_init{false} {}

    template <typename T>
    inline constexpr optional<T>::optional() : m_data{}, m_init{false} {}

    template <typename T>
    inline constexpr optional<T>::optional(T&& data_) : m_data{}, m_init{true} {
        m_data.init(utility::move(data_));
    }

    template <typename T>
    inline constexpr optional<T>::optional(const T& _data) : m_data{}, m_init{true} {
        m_data.init(_data);
    }

    template <typename T>
    inline constexpr optional<T>::optional(optional&& other_) : m_data{}, m_init{other_.m_init} {
        if(m_init) {
            auto& data{other_.m_data};
            m_data.init(utility::move(*data.data()));
            data.fini();
        }
        other_.m_init = false;
    }

    template <typename T>
    inline constexpr optional<T>::optional(const optional& _other) : m_data{}, m_init{_other.m_init} {
        if(m_init) {
            const auto& data{_other.m_data};
            m_data.init(*data.data());
        }
    }

    template <typename T>
    inline optional<T>& optional<T>::operator=(T&& data_) {
        if(m_init) {
            m_data.fini();
        }
        m_init = true;
        m_data.init(utility::move(data_));
        return *this;
    }

    template <typename T>
    inline optional<T>& optional<T>::operator=(const T& _data) {
        if(m_init) {
            m_data.fini();
        }
        m_init = true;
        m_data.init(_data);
        return *this;
    }

    template <typename T>
    inline optional<T>& optional<T>::operator=(optional&& other_) {
        RX_ASSERT(&other_ != this, "self assignment");

        if(m_init) {
            m_data.fini();
        }

        m_init = other_.m_init;

        if(m_init) {
            auto& data{other_.m_data};
            m_data.init(utility::move(*data.data()));
            data.fini();
        }

        other_.m_init = false;

        return *this;
    }

    template <typename T>
    inline optional<T>& optional<T>::operator=(const optional& _other) {
        RX_ASSERT(&_other != this, "self assignment");

        if(m_init) {
            m_data.fini();
        }

        m_init = _other.m_init;

        if(m_init) {
            const auto& data{_other.m_data};
            m_data.init(*data.data());
        }

        return *this;
    }

    template <typename T>
    inline optional<T>::~optional() {
        if(m_init) {
            m_data.fini();
        }
    }

    template <typename T>
    inline optional<T>::operator bool() const {
        return m_init;
    }

    template <typename T>
    inline T& optional<T>::operator*() {
        RX_ASSERT(m_init, "not valid");
        return *m_data.data();
    }

    template <typename T>
    inline const T& optional<T>::operator*() const {
        RX_ASSERT(m_init, "not valid");
        return *m_data.data();
    }

    template <typename T>
    inline T* optional<T>::operator->() {
        RX_ASSERT(m_init, "not valid");
        return m_data.data();
    }

    template <typename T>
    inline const T* optional<T>::operator->() const {
        RX_ASSERT(m_init, "not valid");
        return m_data.data();
    }

    template <typename T>
    const T& optional<T>::value_or(const T& val) const {
        if(m_init) {
            return *m_data.data();
        } else {
            return val;
        }
    }

    template <typename T>
    template <typename OtherType, typename FuncType>
    optional<OtherType> optional<T>::map(FuncType&& map_func) {
        if(m_init) {
            return map_func(*m_data.data());
        } else {
            return nullopt;
        }
    }

    template <typename T>
    template <typename OtherType, typename FuncType>
    optional<OtherType> optional<T>::flat_map(FuncType&& map_func) {
        if(m_init) {
            return *map_func(*m_data.data());
        } else {
            return nullopt;
        }
    }

} // namespace rx

#endif // RX_CORE_OPTIONAL_H
