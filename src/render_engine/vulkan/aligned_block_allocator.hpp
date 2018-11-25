#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include "../../settings/nova_settings.hpp"

namespace nova {
    /*!
     * \brief The memory allocator that Nova uses for mesh data
     *
     * Design goals:
     *  - Store mesh data in a single buffer to facilitate indirect rendering
     *  - Prioritize using less memory over runtime. Typical usage scenario for a game is loading a lot of resources 
     *      when a new level is loaded, but not constantly loading more. Allocation speed isn't the primary concern
     *
     * Adapted from https://www.fasterthan.life/blog/2017/7/13/i-am-graphics-and-so-can-you-part-4- - I've changed it 
     * to ration out parts of a buffer instead of allocating buffers from VkMemoryPools. I've also given it the ability
     * to compact allocations. If an allocation is requested and the total free space in the buffer is greater than or
     * equal to the size of the allocation, but there's no big enough contiguous blocks, memory will be moved to make
     * enough contiguous space
     */
    class compacting_block_allocator {
    public:
        struct allocation_info {
            VkBuffer buffer = VK_NULL_HANDLE;
            uint32_t blockId = 0;
            VkDeviceSize offset = 0;
            VkDeviceSize size = 0;
        };

        /*!
         * \brief A buffer that memory can be allocated from
         */
        class block_allocator_buffer {
            friend class compacting_block_allocator;

        public:
            block_allocator_buffer(uint32_t id, VkDeviceSize size);
            ~block_allocator_buffer();

            bool Init();
            void Shutdown();

            /*!
             * \brief Allocates memory from this buffer
             * 
             * If there's enough space in a single block, memory is allocated from that block. If there is not, but 
             * there is enough total free space, the allocations in this buffer are compacted before allocating the 
             * desired memory. If there is not enough space in this buffer, the method returns false
             * 
             * \param size The size of the allocation that we need
             * \return A pointer to the newly-created allocation, or `nullptr` if the allocation couldn't be made
             */
            allocation_info* allocate(uint32_t size);

            /*!
             * \brief Frees the provided allocation
             * 
             * If the allocation is next to an existing free allocation, it is merged with that one. If it's not next 
             * to a free allocation, 
             */
            void free(allocation_info* alloc);

        private:
            // The pool is a simple linked list of allocated blocks.
            // If a block and its neighbor are free, then they are merged
            // back together in one block.
            struct block_t {
                uint32_t id;
                VkDeviceSize size;
                VkDeviceSize offset;
                block_t* prev;
                block_t* next;
                bool free;
            };
            block_t* head;

            uint32_t id;
            uint32_t next_block_id;
            uint32_t memory_type_index;
            bool host_visible;
            VkBuffer buffer;
            VkDeviceSize size;
            VkDeviceSize allocated;

            void compact_all_memory();
        };

        compacting_block_allocator(settings_options::block_allocator_settings& settings);

        void Init();

        /*!
         * \brief Allocates memory of the requested size and gives that to you
         * 
         * If there's a large enough block of memory in an existing VkBuffer, that block is used
         * If there's enough total space in an existing VkBuffer, but no blocks are large enough, then the allocations 
         * from that VkBuffer are compacted before allocating the memory you requested
         * If there is not enough room in any existing buffers, a new VkBuffer is created and your memory is allocated 
         * from that
         * 
         * \param size The size, in bytes, of the allocation you want
         */
        allocation_info* allocate(uint32_t size);

        /*!
         * \brief Frees a specific allocation
         * 
         * \param allocation The allocation to free
         */
        void free(const allocation_info* allocation);

    private:
        bool AllocateFromPools(const uint32_t size, const uint32_t align, const uint32_t memoryTypeBits, const bool hostVisible, allocation_info& allocation);

    private:
        int m_nextPoolId;
        int m_garbageIndex;

        // How big should each pool be when created?
        int m_deviceLocalMemoryMB;
        int m_hostVisibleMemoryMB;

        std::vector<block_allocator_buffer> m_pools;
    };

}  // namespace nova
