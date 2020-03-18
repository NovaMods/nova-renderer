#pragma once

#include <rx/core/memory/allocator.h>
#include <stdint.h>

namespace nova::renderer {
    /*!
     * \brief Array that can hold data of multiple types of multiple sizes
     *
     * This array uses a linear allocator internally
     */
    class MaterialDataBuffer {
    public:
        explicit MaterialDataBuffer(rx::memory::view buffer);

        MaterialDataBuffer(const MaterialDataBuffer& other) = delete;
        MaterialDataBuffer& operator=(const MaterialDataBuffer& other) = delete;

        MaterialDataBuffer(MaterialDataBuffer&& old) noexcept = default;
        MaterialDataBuffer& operator=(MaterialDataBuffer&& old) noexcept = default;

        ~MaterialDataBuffer() = default;

        /*!
         * \brief Provides access to an element in this array
         *
         * This operator performs no checks that the requested element is of the requested type. I recommend that you only use indices you
         * get from `get_next_free_index` with the same type as what you're requesting
         */
        template <typename MaterialDataStruct>
        [[nodiscard]] MaterialDataStruct& at(uint32_t idx);

        /*!
         * \brief Provides access to an element in this array
         *
         * This operator performs no checks that the requested element is of the requested type. I recommend that you only use indices you
         * get from `get_next_free_index` with the same type as what you're requesting
         */
        template <typename MaterialDataStruct>
        [[nodiscard]] const MaterialDataStruct& at(uint32_t idx) const;

        /*!
         * \brief Gets the index of the next free element of the requested type
         */
        template <typename MaterialDataStruct>
        [[nodiscard]] uint32_t get_next_free_index();

        [[nodiscard]] rx_byte* data() const;

    private:
        rx::memory::view buffer;

        uint32_t num_allocated_bytes = 0;
    };

    template <typename MaterialDataStruct>
    MaterialDataStruct& MaterialDataBuffer::at(uint32_t idx) {
        return reinterpret_cast<MaterialDataStruct*>(buffer.data)[idx];
    }

    template <typename MaterialDataStruct>
    const MaterialDataStruct& MaterialDataBuffer::at(uint32_t idx) const {
        return reinterpret_cast<MaterialDataStruct*>(buffer.data)[idx];
    }

    template <typename MaterialDataStruct>
    uint32_t MaterialDataBuffer::get_next_free_index() {
        constexpr uint32_t struct_size = sizeof(MaterialDataStruct);

        // Here's a Al Gore rhythm for your soul

        // This class is a party. The idea is that it's an array of any type you want. You reinterpret the buffer pointer to the type you
        // want at runtime
        //
        // So like if you want to store five floats, one float3, and a float4x4 all in the same buffer... you can do that, and they each get
        // an index. They get an index as if the buffer was an array of their type? So when we find a place to put them in the buffer - aka
        // in this method - we have to align the number of already-allocated bytes to the size of the struct of the new material, rounding
        // up. This means that we end up with a lot of empty bytes here any there. Ideally we can find a way to force alignment on material
        // structs and avoid wasting _too_ much data, but who knows

        // Intentionally using integer division
        const auto new_idx = (num_allocated_bytes / struct_size) + 1;

        num_allocated_bytes = struct_size * new_idx;

        return new_idx;
    }
} // namespace nova::renderer
