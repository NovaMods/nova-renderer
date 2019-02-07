#pragma once

#include "../../util/vma_usage.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include "../../settings/nova_settings.hpp"
#include "../../util/utils.hpp"
#include <mutex>

namespace ftl {
    class TaskScheduler;
}

namespace nova {
    namespace ttl {
        class task_scheduler;
    }

    NOVA_EXCEPTION(buffer_allocation_failed);

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
        class block_allocator_buffer;

        struct allocation_info {
            block_allocator_buffer* block = nullptr;
            uint32_t block_id = 0;
            VkDeviceSize offset = 0;
            VkDeviceSize size = 0;
        };

        /*!
         * \brief A buffer that memory can be allocated from
         */
        class block_allocator_buffer {
            friend class compacting_block_allocator;

        public:
            /*!
             * \brief Initializes a new block_allocator_buffer
             */
            block_allocator_buffer(VkDeviceSize size, VmaAllocator allocator);

            block_allocator_buffer(const block_allocator_buffer& other) = delete;
            block_allocator_buffer(block_allocator_buffer&& other) noexcept;

            block_allocator_buffer& operator=(const block_allocator_buffer& other) = delete;
            block_allocator_buffer& operator=(block_allocator_buffer&& other) noexcept;

            ~block_allocator_buffer();

            /*!
             * \brief Allocates memory from this buffer
             *
             * If there's enough space in a single block, memory is allocated from that block. If there is not, but
             * there is enough total free space, the allocations in this buffer are compacted before allocating the
             * desired memory. If there is not enough space in this buffer, the method returns false
             *
             * \param needed_size The size of the allocation that we need
             * \return A pointer to the newly-created allocation, or `nullptr` if the allocation couldn't be made
             */
            allocation_info* allocate(VkDeviceSize needed_size);

            /*!
             * \brief Frees the provided allocation
             *
             * If the allocation is next to an existing free allocation, it is merged with that one. If it's not next
             * to a free allocation,
             */
            void free(allocation_info* alloc);

            VkBuffer get_buffer() const;

        private:
            // The pool is a simple linked list of allocated blocks.
            // If a block and its neighbor are free, then they are merged
            // back together in one block.
            //
            // Critical note: The linked list is kept sorted
            struct block_t {
                uint32_t id = 0;
                VkDeviceSize size = 0;
                VkDeviceSize offset = 0;
                block_t* prev = nullptr;
                block_t* next = nullptr;
                bool free = true;
            };
            block_t* head;

            VmaAllocator allocator;

            static uint32_t next_id;
            uint32_t id;

            uint32_t next_block_id = 0;
            VkBuffer buffer = VK_NULL_HANDLE;
            VmaAllocation vma_allocation;
            VmaAllocationInfo vma_allocation_info;
            VkDeviceSize size;
            VkDeviceSize allocated = 0;

            allocation_info* allocate_internal(VkDeviceSize needed_size, bool can_compact);

            void compact_all_memory();
        };

        compacting_block_allocator(const settings_options::block_allocator_settings& settings, VmaAllocator vma_allocator, uint32_t graphics_queue_idx, uint32_t copy_queue_idx);

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
        allocation_info* allocate(VkDeviceSize size);

        /*!
         * \brief Frees a specific allocation
         *
         * \param allocation The allocation to free
         */
        void free(allocation_info* allocation);

        /*!
         * \brief Adds barriers to the provided command buffer to ensure that reading vertex data has finished before transfers
         * 
         * \param cmds The command buffer to add commands to
         */
        void add_barriers_before_data_upload(VkCommandBuffer cmds) const;

        /*!
         * \brief Adds barriers to the provided command buffer to ensure that transfers are done before reading vertex data has started
         * 
         * \param cmds The command buffer to add commands to
         */
        void add_barriers_after_data_upload(VkCommandBuffer cmds) const;

    private:
        std::vector<block_allocator_buffer> pools;
        std::mutex pools_mutex;

        const settings_options::block_allocator_settings settings;
        VmaAllocator vma_allocator;
        uint32_t graphics_queue_idx;
        uint32_t copy_queue_idx;
    };

}  // namespace nova
