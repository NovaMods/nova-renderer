/*!
 * \author ddubois 
 * \date 17-Oct-17.
 */

#ifndef RENDERER_DEVICE_MEMORY_ALLOCATOR_H
#define RENDERER_DEVICE_MEMORY_ALLOCATOR_H

#include <vulkan/vulkan.hpp>

namespace nova {
    /*!
     * \brief An allocation of memory that can be used by something
     */
    struct allocation {
        allocation() :
                id(0),
                block_id(0),
                data(nullptr)
        {}

        uint32_t id;
        uint32_t block_id;
        vk::DeviceMemory device_memory;
        vk::DeviceSize offset;
        vk::DeviceSize size;
        void* data;
    };

    class memory_pool {
    public:
        memory_pool(const uint32_t id, const uint32_t memoty_type_bits, const vk::DeviceSize size, const bool is_host_visible);
        ~memory_pool();

        bool init();
        bool shutdown();

        bool allocate(const uint32_t size, const uint32_t align, allocation& allocation);
        void free(allocation& allocation);

        // Pools are linked lists of blocks. We can easily enough merge blocks if they're next to each other and free
        struct block {
            uint32_t id;
            vk::DeviceSize size;
            vk::DeviceSize offset;
            block* prev;
            block* next;
            bool free;
        };

        block* head;

        uint32_t id;
        uint32_t next_block_id;
        uint32_t memory_type_index;
        bool host_visible;
        vk::DeviceMemory device_memory;
        vk::DeviceSize size;
        vk::DeviceSize allocated;
        void* data;
    };

    /*!
     * \brief Handles allocation of GPU memory
     *
     * Allocation operations:
     * - Allocate the virtual texture at resource pack load. The size of the virtual texture must be carefully chosen and should
     *      scale with resourcepack resolution
     *      - Infrequent (resourcepack reload)
     *      - Large block of memory
     * - Allocate descriptor sets for shaders
     *      - Infrequent (shaderpack reload)
     *      - Could have one allocator per thread, enabling multithreaded shader loading (could be good for big ass
     *          shaderpacks)
     * - Allocate chunks memory when a new chunk is loaded. Chunks know how much memory their buffers need
     *      - Frequent (chunks are loaded in as the player explores the world)
     *      - 20k verts * 64 bytes per vert = ~1.28m bytes for the vertex buffer
     *      - 20k verts * 2 bytes per index = 40k bytes for indices
     *      - So like we have 1.32 MB per chunk, which seems super high. That only allows 6060 chunks on an 8 GB card,
     *          render distance of like 38 assuming no textures
     *      - Should probably have one allocator per thread
     * - Allocate memory for entities
     *      - Each entity needs separate animation memory but can share a model
     *      - per-object animation buffer, per-entity-type vertex/index buffer
     *      - Textures use the virtual textures system
     *      - Frequent (entities are loaded as the player explores the world)
     * - Allocate memory for particles
     *      - Best way to handle particles is probably a large vertex buffer per particle system
     *      - Each dripping lava systems can be shared, dripping water systems can be shared, etc
     *      - Basically I think imma make one vertex buffer per particle system type and one animation data buffer per
     *          actual particle system, like entities
     *      - Hopefully I can pool particle system animation buffers like entity animation buffers. If I can divide
     *          particle systems into a few groups based on how many particles they have then we can all party
     *      - Can update particle system animations in compute shaders, which can be easily multithreaded (I just need
     *          them all to finish before I render the frame)
     *      - Can hopefully allocate a few pooled objects up front, but might need more if there's a lot happening
     * - I'm still not quite sure how command buffers work, but I'm told I should have a pool of them for each thread
     *      so I will
     *      - Do these happen per frame? Can I reuse them?
     *
     *
     * Deallocation operations:
     *
     * - Deallocate chunk memory when a chunk is unloaded
     *      - Frequent (chunks removed as players leave them behind)
     * - Deallocate entity animation buffers
     *      - Frequent (Entities are unloaded as the player leaves them behind)
     *      - Actually, since all entities of a given type have animation buffers of the same size, entity animation
     *          buffers can be reused. They should be pooled in a pool that scales as big as needed
     * - No need to deallocate virtual textures, they're active for the entire application lifetime
     *      - If a new virtual texture atlas is needed during gameplay, we can leave it active since it might be needed
     *          again
     *      - should collect usage statistics on additional virtual texture buffers
     *
     * I have one class to handle creating resources and assigning them memory. The tutorial I'm following has separate
     * classes. I'm sure there's something about cohesion and coupling in there somewhere but imma do things like this
     * until I can't
     */
    class device_memory_allocator {
    public:
        static device_memory_allocator& get_instance();

        device_memory_allocator() noexcept;

        void init();    // Why is this separate from the contructor?

        allocation allocate(const uint32_t size, const uint32_t align, const uint32_t memoty_type_bits, const bool host_visible);

        void free(const allocation& allocation);

        /*!
         * \brief Frees all allocations that are part of the garbage list
         *
         * Maybe I want to add a few resources to a list of garbage, then
         */
        void empty_garbage();

    private:
        static device_memory_allocator instance;

        uint32_t next_pool_id;
        uint32_t garbage_index;
        uint64_t device_local_memory_mb;
        uint64_t host_visible_memory_mb;

        std::vector<memory_pool*> pools;
        std::vector<allocation> garbage[2];

        bool allocate_from_pools(const uint32_t size, const uint32_t align, const uint32_t memory_type_bits, const bool need_host_visible, allocation& alloc);
    };
}

#endif //RENDERER_DEVICE_MEMORY_ALLOCATOR_H
