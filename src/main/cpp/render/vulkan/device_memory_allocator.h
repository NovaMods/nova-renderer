/*!
 * \author ddubois 
 * \date 17-Oct-17.
 */

#ifndef RENDERER_DEVICE_MEMORY_ALLOCATOR_H
#define RENDERER_DEVICE_MEMORY_ALLOCATOR_H

namespace nova {
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
     *      - 20k verts * 56 bytes per vert = ~100k bytes for the vertex buffer
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
     */
    class device_memory_allocator {

    };
}

#endif //RENDERER_DEVICE_MEMORY_ALLOCATOR_H
