#pragma once

#include <vector>

namespace nova::renderer {
    /*!
     * \brief Allows one to access a thing at a location in a vector, even after the vector has been reallocated
     *
     * How this isn't in the standard library, idk
     */
    template<typename T>
    class VectorAccessor {
    public:
        VectorAccessor(const std::vector<T>& vec, const std::size_t idx) : vec(vec), idx(idx) {}

        T* operator->() {
            return &vec[idx];
        }

    private:
        std::vector<T>& vec;
        std::size_t idx;
    };
}
