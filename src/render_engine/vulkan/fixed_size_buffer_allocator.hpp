#pragma once

#include <cstdint>

#include "uniform_buffer.hpp"

namespace nova::renderer {
    /*!
     * \brief Allocates fixed-size blocks from a uniform buffer
     *
     * Blocks are an index into the underlying VkBuffer. This tells you where in the VkBuffer you should put your data
     *
     * Usage notes:
     * The destructor assumes that every allocation has been returned to the pool. If there are active allocations at
     * the time the `fixed_size_block_allocator` is destroyed, they will become invalid and their memory may be reused.
     * Be careful.
     *
     * \tparam BlockSize The size, in bytes, of one block
     */
    template <uint32_t BlockSize>
    class fixed_size_buffer_allocator : public uniform_buffer {
    public:
        struct block {
            uint32_t index = 0;

        private:
            block* next = nullptr;
        };

        /*!
         * \brief Allocates the uniform buffer and sets up block allocation info
         *
         * \param name The name of this uniform buffer
         * \param allocator the VmaAllocator you're allocating the uniform buffer from
         * \param create_info the VkBufferCreateInfo for the uniform buffer you want to create
         * \param alignment The alignment, in bytes, of the uniform buffer. This can be gotten from your
         * VkPhysicalDeviceProperties struct
         * \param mapped If true, the uniform buffer is constantly mapped
         */
        fixed_size_buffer_allocator(const std::string& name,
                                    VmaAllocator allocator,
                                    const VkBufferCreateInfo& create_info,
                                    const uint64_t alignment,
                                    const bool mapped = false);

        fixed_size_buffer_allocator(const fixed_size_buffer_allocator& other) = delete;
        fixed_size_buffer_allocator& operator=(const fixed_size_buffer_allocator& other) = delete;

        fixed_size_buffer_allocator(fixed_size_buffer_allocator&& old) noexcept;
        fixed_size_buffer_allocator& operator=(fixed_size_buffer_allocator&& old) noexcept;

        virtual ~fixed_size_buffer_allocator();

        /*!
         * \brief Allocates a single block from the buffer
         *
         * Blocks
         *
         * \return The newly allocated block
         */
        block* allocate_block();

        void free_block(block* freed_block);

    private:
        block* blocks;

        uint32_t num_blocks;
    };
} // namespace nova::renderer
