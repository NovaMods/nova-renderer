#pragma once

#include <cstdint>

#include "nova_renderer/allocation_strategy.hpp"
#include "nova_renderer/bytes.hpp"
#include "nova_renderer/polyalloc.hpp"

namespace bvestl {
    namespace polyalloc {
        struct AllocationInfo;

        /*!
         * \brief An block allocation strategy
         *
         * Block allocators support both allocation and deallocation. They maintain an internal list of free memory
         * regions, passing information about newly-allocated memory regions in the data section of `AllocationInfo`.
         * They're great for like an object pool or somewhere else you'd be freeing memory from
         */
        class BlockAllocationStrategy final : public AllocationStrategy {
            // Basically the allocator from https://www.fasterthan.life/blog/2017/7/13/i-am-graphics-and-so-can-you-part-4- but I've changed
            // it so my allocator only deals with sizes and offsets, and doesn't care about memory types at all
        public:
            struct Block {
                uint64_t id = 0;

                Bytes size{0};
                Bytes offset{0};

                Block* previous = nullptr;
                Block* next = nullptr;

                bool free = true;
            };

            /*!
             * \brief Initializes this allocator with the total size of the memory it can work with
             *
             * \param allocator_in The allocator to use when allocating internally
             * \param size The size of the memory that this boi can allocate from
             * \param alignment_in The alignment of all allocations from this allocator
             */
            BlockAllocationStrategy(const allocator_handle& allocator_in, Bytes size, Bytes alignment_in = Bytes(0));

            ~BlockAllocationStrategy();

            /*!
             * \brief Allocates the specified amount of memory, filling out `allocation` if the allocation is successful
             *
             * This method finds the first free block that's large enough to allocate from. If the block is larger than the requested size,
             * it's shrunk to the requested and a new block is created to represent the free space
             *
             * \param size The size of your allocation
             * \param allocation The struct to fill out with information about the allocation
             * \return True if the allocation succeeds, false otherwise. The allocation can only fail if you're out of memory
             */
            bool allocate(Bytes size, AllocationInfo& allocation) override;

            void free(const AllocationInfo& alloc) override;

        private:
            allocator_handle allocator;

            Block* head;

            Bytes memory_size{0};
            Bytes alignment{0};

            Bytes allocated{0};

            uint64_t next_block_id = 0;

            Block* make_new_block(Bytes offset, Bytes size);
        };
    } // namespace polyalloc
} // namespace bvestl
