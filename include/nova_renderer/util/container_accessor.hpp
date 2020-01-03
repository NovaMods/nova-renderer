#pragma once

namespace nova::renderer {
    /*!
     * \brief Allows one to access a thing at a location in a vector, even after the vector has been reallocated
     *
     * How this isn't in the standard library, idk
     */
    template <typename KeyType, typename ValueType>
    class MapAccessor {
    public:
        MapAccessor(const std::unordered_map<KeyType, ValueType>* map, const KeyType& key) : map(map), key(key) {}

        MapAccessor(const MapAccessor& other) = default;
        MapAccessor& operator=(const MapAccessor& other) = default;

        MapAccessor(MapAccessor&& old) noexcept = default;
        MapAccessor& operator=(MapAccessor&& old) noexcept = default;

        ~MapAccessor() = default;

        [[nodiscard]] ValueType* operator->() const { return &map->at(key); }

        [[nodiscard]] const KeyType& get_key() const { return key; }

    private:
        std::unordered_map<KeyType, ValueType>* map;
        KeyType key;
    };
} // namespace nova::renderer
