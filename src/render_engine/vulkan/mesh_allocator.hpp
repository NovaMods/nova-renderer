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
#include "../render_engine.hpp"

namespace nova {
    struct buffer_range {
        VkBuffer buffer;
        uint32_t offset;
        // Don't need to store the size, wince we can look at the global constant `buffer_part_size`
    };

    struct mesh_memory {
        std::vector<buffer_range> parts;
        uint64_t allocated_size = 0;
    };
    
    /*!
     * \brief Stores all the mesh data the Nova uses
     * 
     * Nova's mesh data isn't what most folk think of as mesh data. Rather than have one buffer for each object, Nova 
     * has one buffer per 64 MB of objects. This allows it to drastically reduce the number of drawcalls needed for 
     * anything
     * 
     * When a mesh is added, the mesh automatically gets a 16k block of memory (this may change later after some 
     * testing). It can get more than one if it's a larger mesh - so meshes can be bigger than 16k if they want to.
     * The idea here is that as chunks grow and shrink when the player breaks and places blocks, the chunk mesh can
     * add and remove buffer parts as needed
     * 
     * Why put lots of things in a large buffer? Nova uses indirect drawing, and putting everything in one big mesh 
     * lets you do that. When a mesh gets marked as visible or invisible, its visibility status gets updated and it
     * stops being drawn
     */
    class mesh_allocator {
    public:
        const static uint32_t new_buffer_size = 16 * 1024 * 1024 * sizeof(full_vertex);  // 16m vertices
        const static uint32_t buffer_part_size = 16 * 1024 * sizeof(full_vertex);    // 16k vertices

        /*!
         * \brief Creates a new mesh store. A single physical buffer is created and made ready for use
         * 
         * \param max_size The maximum size, in bytes, that this mesh_allocator is allowed to grow to
         * \param alloc The device memory allocator to allocate new buffers with
         * \param task_scheduler Nova's global task scheduler, which it used here to create fibtexes to synchronize 
         * access to the physical buffers
         */
        mesh_allocator(uint64_t max_size, const VmaAllocator* alloc, ftl::TaskScheduler* task_scheduler);

        // Copying is for squares

        mesh_allocator(const mesh_allocator& other) = delete;

        mesh_allocator& operator=(const mesh_allocator& other) = delete;

        // Moving is for galaxy brains

        mesh_allocator(mesh_allocator&& other) = default;

        mesh_allocator& operator=(mesh_allocator&& other) noexcept = default;

        /*!
         * \brief Deletes the physical buffers
         * 
         * Using any memory gotten from an instance of this class after that instance has been destructed will result
         * in undefined behavior so don't do it
         */
        ~mesh_allocator();

        /*!
         * \brief Allocates a bunch of buffer space for the new mesh
         * 
         * If memory can be stored in existing buffers that's awesome, otherwise a new VkBuffer will be allocated, and
         * used for the new mesh's parts. The parts may be in separate buffers and may not be completely congruent! 
         * This code has no qualms about spreading your mesh over multiple buffers
         * 
         * \param num_verts The number of vertices that the new mesh has
         * 
         * \return All the information you need to know about the memory for your mesh
         */
        mesh_memory allocate_mesh(uint64_t num_verts);

        /*!
         * \brief Frees the mesh, returning it to the pool
         * 
         * This method currently does not free any VkBuffer objects. If your mesh memory usage suddenly spikes, too 
         * bad. You're stuck with that now
         * 
         * \param memory_to_free The mesh memory to free. Usage of that mesh memory after calling this function is not 
         * valid usage
         */
        void free_mesh(const mesh_memory& memory_to_free);

        /*!
         * \brief Retrieves the total number of vertices that can fit in all the buffers owned by this mesh_allocator
         */
        uint64_t get_total_vertex_capacity() const;

        /*!
         * \brief Gets the count of vertices that are currently being used by a mesh
         */
        uint64_t get_allocated_vertex_count() const;

        /*!
         * \brief Gets the number of vertices that can fit into unused space in any of the buffers
         */
        uint64_t get_available_vertex_count() const;

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

        /*
         * \brief Allocated a new buffer to allocate mesh memory out of
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
