/*!
 * \author ddubois 
 * \date 11-Nov-18.
 */

#ifndef NOVA_RENDERER_MESH_STORE_HPP
#define NOVA_RENDERER_MESH_STORE_HPP

#include <vector>
#include <unordered_map>
#include <vk_mem_alloc.h>
#include "ftl/fibtex.h"
#include "ftl/atomic_counter.h"
#include "../../settings/nova_settings.hpp"

namespace nova {
    struct buffer_range {
        VkBuffer buffer;
        uint32_t offset;
        // Don't need to store the size, wince we can look at the global constant `buffer_part_size`
    };

    struct block_memory_allocation {
        std::vector<buffer_range> parts;
        uint64_t allocated_size = 0;
    };
    
    /*!
     * \brief A block allocator, with the alignment as a template parameter
     */
    class block_allocator {
    public:
        uint32_t new_buffer_size;
        uint32_t buffer_part_size;
        
        /*!
         * \brief Creates a new block allocator. A single physical buffer is created and made ready for use
         * 
         * \param options The settings for this block_allocator to use
         * \param alloc The device memory allocator to allocate new buffers with
         * \param task_scheduler The TaskScheduler that we should use to make mutexes and whatever else this class needs
         * \param graphics_queue_idx The index of the graphics queue where the meshes allocated from this pool will be used
         * \param copy_queue_idx The index of the transfer queue where meshes allocated from this pool will be used
         */
        block_allocator(const settings_options::mesh_options& options, const VmaAllocator* alloc, ftl::TaskScheduler* task_scheduler, uint32_t graphics_queue_idx, uint32_t copy_queue_idx);
        
        /*!
         * \brief Deletes the physical buffers
         * 
         * Using any memory gotten from an instance of this class after that instance has been destructed will result
         * in undefined behavior so don't do it
         */
        ~block_allocator();

        /*!
         * \brief Allocates a bunch of buffer space for the new data
         * 
         * If memory can be stored in existing buffers that's awesome, otherwise a new VkBuffer will be allocated, and
         * used for the new allocation's parts. The parts may be in separate buffers and may not be completely congruent! 
         * This code has no qualms about spreading your allocation over multiple buffers
         * 
         * \param size The size, in bytes, of the needed allocation
         * 
         * \return All the information you need to know about the memory for your allocation
         */
        block_memory_allocation allocate(uint64_t size);

        /*!
         * \brief Frees the allocation, returning it to the pool
         * 
         * This method currently does not free any VkBuffer objects. If your memory usage suddenly spikes, too 
         * bad. You're stuck with that now
         * 
         * \param memory_to_free The allocation to free. Usage of that allocation's memory after calling this function is not 
         * valid usage
         */
        void free(const block_memory_allocation& memory_to_free);

        uint64_t get_num_bytes_allocated() const;

        uint64_t get_num_bytes_used() const;

        uint64_t get_num_bytes_available() const;

        /*!
         * \brief Adds barriers for all our buffers to ensure that reads are done before we upload new data
         * 
         * \param cmds The command buffer to record the barriers into
         */
        void add_barriers_before_data_upload(VkCommandBuffer cmds);

        /*!
         * \brief Adds barriers to all our buffers to ensure that the data upload is done before any reads
         * 
         * \param cmds The command buffer to record the barriers into
         */
        void add_barriers_after_data_upload(VkCommandBuffer cmds);

    private:
        struct mega_buffer_info {
            VmaAllocation allocation;
            VmaAllocationInfo alloc_info;
            std::vector<buffer_range> available_ranges;
        };

        const VmaAllocator* vma_alloc;

        ftl::Fibtex buffer_fibtex;
        std::unordered_map<VkBuffer, mega_buffer_info> buffers;
        uint64_t max_size;
        uint32_t graphics_queue_idx;
        uint32_t copy_queue_idx;

        /*
         * \brief Allocates a new buffer to allocate memory out of
         * 
         * Allocates a buffer with a size of `new_buffer_size`. That buffer is in device local memory, you'll have to 
         * transfer to it from a transfer queue. Be sure to only use the transfer queue that's in 
         * `vulkan_render_device`. You also have to handle all synchronization yourself, ensuring that there's the 
         * proper cross-queue barriers so we don't try rendering with this thing while we're still writing to it
         * 
         * Good luck!
         * 
         * \return The buffer and info that were just created
         */
        std::pair<VkBuffer, mega_buffer_info&> allocate_new_buffer();

        /*!
        * \brief Allocates a buffer range
        *
        * If there's space in an existing buffer, that range is removed from the buffer's list of ranges and is
        * returned
        *
        * If there's not space, a new buffer is allocated and one of its parts is returned
        */
        buffer_range get_buffer_part();
    };
}

#endif //NOVA_RENDERER_MESH_STORE_HPP
