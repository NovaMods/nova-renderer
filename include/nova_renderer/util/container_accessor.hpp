#pragma once

#include <rx/core/map.h>

namespace nova::renderer {
    /*!
     * \brief Allows one to access a thing at a location in a vector, even after the vector has been reallocated
     *
     * How this isn't in the standard library, idk
     */
    template <typename KeyType, typename ValueType>
    class MapAccessor {
    public:
        MapAccessor() = default;

        MapAccessor(std::unordered_map<KeyType, ValueType> * map, const KeyType& key) : map(map), key(key) {}

        MapAccessor(const MapAccessor& other) = default;
        MapAccessor& operator=(const MapAccessor& other) = default;

        MapAccessor(MapAccessor&& old) noexcept = default;
        MapAccessor& operator=(MapAccessor&& old) noexcept = default;

        ~MapAccessor() = default;

        [[nodiscard]] const ValueType* operator->() const { return map->find(key); }

        [[nodiscard]] ValueType* operator->() { return map->find(key); }

        [[nodiscard]] const KeyType& get_key() const { return key; }

    private:
        std::unordered_map<KeyType, ValueType>* map = nullptr;
        KeyType key = {};
    };

    template <typename ValueType>
    class VectorAccessor {
    public:
        VectorAccessor() = default;

        VectorAccessor(std::vector<ValueType>* vec, const size_t idx) : vec(vec), idx(idx) {}

        VectorAccessor(const VectorAccessor& other) = default;
        VectorAccessor& operator=(const VectorAccessor& other) = default;

        VectorAccessor(VectorAccessor&& old) noexcept = default;
        VectorAccessor& operator=(VectorAccessor&& old) noexcept = default;

        ~VectorAccessor() = default;

        [[nodiscard]] const ValueType* operator->() const { return &(*vec)[idx]; }

        [[nodiscard]] ValueType* operator->() { return &(*vec)[idx]; }

        [[nodiscard]] const ValueType& operator*() const { return (*vec)[idx]; }

        [[nodiscard]] const size_t& get_idx() const { return idx; }

    private:
        std::vector<ValueType>* vec = nullptr;
        size_t idx = 0;
    };
} // namespace nova::renderer
