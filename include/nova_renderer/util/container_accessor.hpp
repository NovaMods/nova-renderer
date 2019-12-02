#pragma once

#include <vector>

namespace nova::renderer {
    /*!
     * \brief Allows one to access a thing at a location in a vector, even after the vector has been reallocated
     *
     * How this isn't in the standard library, idk
     */
    template<typename KeyType, typename ValueType>
    class MapAccessor {
    public:
        MapAccessor(std::unordered_map<KeyType, ValueType>& map, const KeyType key) : map(map), key(key) {}

        ValueType* operator->() {
            return &map[key];
        }

        KeyType get_key() const {
            return key;
        }

    private:
        std::unordered_map<KeyType, ValueType>& map;
        KeyType key;
    };
}
