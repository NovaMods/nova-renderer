#pragma once

#include <cstdint>

#include "result.hpp"

//! \brief Beautiful vectors!

namespace ntl {
    /*!
     * \brief A vector whose capacity may not change after initialization
     *
     * \tparam ValueType Type of the data that this vector stores. Must be movable
     * \tparam AllowResizing If true, this vector can resize at runtime. If false, it's a fixed-capacity vector
     */
    template <typename ValueType, bool AllowResizing = true>
    class Vector {
    public:
        Vector() : m_buffer(new ValueType[11]), m_capacity(11) {}

        /*!
         * \brief Initializes the fixed size vector with the provided capacity
         *
         * This construct will allocate a buffer of the desired size, but it will not fill in the buffer with anything useful
         *
         *\param capacity The capacity of your vector
         */
        explicit Vector(uint64_t const capacity) : m_buffer(new ValueType[capacity]), m_capacity(capacity) {}

        /*!
         * \brief Initializes this fixed size vector from an arbitrary buffer
         *
         * This constructor takes over management of the buffer. It will free the buffer when the fixed size vector is destructed
         *
         * \param buffer The buffer which stores the array to wrap
         * \param length The number of elements in the array
         */
        Vector(ValueType* const buffer, uint64_t const length) : m_buffer(buffer), m_capacity(length), m_size(length) {}

        explicit Vector(Vector&& old) noexcept : m_buffer(old.m_buffer), m_capacity(old.m_capacity), m_size(old.m_size) { old.m_size = 0; }
        Vector& operator=(Vector&& old) noexcept {
            m_buffer = old.m_buffer;
            m_capacity = old.m_capacity;

            old.m_size = 0;

            return *this;
        }

        explicit Vector(Vector const& other)
            : m_buffer(new ValueType[other.m_capacity]), m_capacity(other.m_capacity), m_size(other.m_size) {
            std::memcpy(m_buffer, other.m_buffer, m_capacity * sizeof(ValueType));
        }
        Vector& operator=(const Vector& other) {
            m_buffer = new ValueType[other.m_capacity];
            m_capacity = other.m_capacity;

            std::memcpy(m_buffer, other.m_buffer, m_capacity * sizeof(ValueType));

            return *this;
        }

        ~Vector() {
            if(m_size > 0) {
                delete[] m_buffer;
            }
        }

        /*!
         * \brief Retrieves the current length of the fixed capacity vector
         *
         * The length is the number of elements currently in the vector
         */
        [[nodiscard]] uint64_t length() const { return m_size; }

        /*!
         * \brief Retrieves the current capacity of the vector
         *
         * The capacity is the number of elements which can possible fit into this fixed size vector
         */
        [[nodiscard]] uint64_t capacity() const { return m_capacity; }

        /*!
         * \brief Returns a reference to the first element in this vector
         */
        [[nodiscard]] Result<ValueType&, bool> first() const {
            if(m_size > 0) {
                return Result(m_buffer[0]);

            } else {
                return Result(false);
            }
        }

        /*!
         * \brief Returns a reference to the last element in this vector
         */
        [[nodiscard]] Result<ValueType&, bool> last() const {
            if(m_size > 0) {
                return Result(m_buffer[m_size - 1]);

            } else {
                return Result(false);
            }
        }

        /*!
         * \brief Returns a reference to the element at the specified index
         *
         * \param idx The index of the element you want
         */
        [[nodiscard]] Result<ValueType&, bool> at(uint64_t const idx) const {
            if(idx < m_capacity) {
                return Result(m_buffer[idx]);

            } else {
                return Result(false);
            }
        }
        
        /*!
         * \brief Pushes a new object onto the vector
         *
         * \return True if we could push the object onto the vector, false if the vector is already full
         */
        bool push(ValueType&& new_value) {
            if(m_size == m_capacity) {
                if(AllowResizing) {
                    grow_buffer();

                } else {
                    return false;
                }
            }

            m_size++;
            m_buffer[m_size] = new_value;

            return true;
        }

        /*!
         * \brief Moves the last element in the vector out of the vector and out to your code
         */
        [[nodiscard]] Result<ValueType, bool> pop() {
            if(m_size != 0) {
                m_size--;

                return Result(std::move(m_buffer[m_size + 1]));

            } else {
                return Result(false);
            }
        }

        [[nodiscard]] Iterator<ValueType> begin() const noexcept {}

        [[nodiscard]] Iterator<ValueType> end() const noexcept {}

        /*!
         * \brief Returns a stream of this vector's elements, allowing you to write complex functional pipelines that process this vector
         */
        // [[nodiscard]] Stream const & stream() const;

    private:
        ValueType* m_buffer;

        uint64_t m_capacity;
        uint64_t m_size = 0;

        /*!
         * \brief Doubles the size of the internal buffer
         */
        void grow_buffer() { auto* new_buffer = new ValueType[m_capacity * 2];

            std::memcpy(new_buffer, m_buffer, m_size * sizeof(ValueType));

            delete[] m_buffer;

            m_buffer = new_buffer;
            m_capacity *= 2;
        }
    };
} // namespace ntl
