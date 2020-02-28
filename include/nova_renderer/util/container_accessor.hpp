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
        MapAccessor(rx::map<KeyType, ValueType>* map, const KeyType& key) : map(map), key(key) {}

        MapAccessor(const MapAccessor& other) = default;
        MapAccessor& operator=(const MapAccessor& other) = default;

        MapAccessor(MapAccessor&& old) noexcept = default;
        MapAccessor& operator=(MapAccessor&& old) noexcept = default;

        ~MapAccessor() = default;

        [[nodiscard]] const ValueType* operator->() const { return map->find(key); }

        [[nodiscard]] ValueType* operator->() { return map->find(key); }

        [[nodiscard]] const KeyType& get_key() const { return key; }

    private:
        rx::map<KeyType, ValueType>* map;
        KeyType key;
    };
} // namespace nova::renderer
