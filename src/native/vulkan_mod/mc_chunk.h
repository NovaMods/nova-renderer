#ifndef VULKAN_MOD_MC_CHUNK_H
#define VULKAN_MOD_MC_CHUNK_H

// FILE: C:/Users/David/Documents/MC/force-src/src/main/native/vulkan_mod/mc_chunk.h

#include "ivertex_buffer.h"

namespace vulkan_mod {
class store;
} /* End of namespace vulkan_mod */

namespace vulkan_mod {


    /** 
     *  Represents a chunk from Minecraft.
     *  
     *  Chunks should be updated from Minecraft every render loop. I can make this pull data from Minecraft's RenderChunk maybe?
     */
class mc_chunk {

    // Operations
 public:


    /** 
     *  Returns this chunk's vertex buffer
     */
    virtual ivertex_buffer get_vertex_buffer();

    // Attributes
 public:
    int id;
    ivertex_buffer vertex_buffer;

    // Associations
 public:

    /**
     * @element-type store
     */
    store *mystore;

    /**
     * @element-type ivertex_buffer
     */
    ivertex_buffer *myivertex_buffer;

//end of class mc_chunk
};

} /* End of namespace vulkan_mod */

#endif // VULKAN_MOD_MC_CHUNK_H
