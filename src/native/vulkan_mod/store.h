#ifndef VULKAN_MOD_STORE_H
#define VULKAN_MOD_STORE_H

// FILE: C:/Users/David/Documents/MC/force-src/src/main/native/vulkan_mod/store.h

#include <vector>

#include "ishader.h"
#include "itexture.h"
#include "ivertex_buffer.h"
#include "mc_chunk.h"
#include "mc_entity.h"

namespace vulkan_mod {
class render_data_store;
} /* End of namespace vulkan_mod */

namespace vulkan_mod {

class store {
    /* {deprecated=false}*/


    // Operations
 public:

    virtual void add_item(T item);

    virtual void remove_item(void  id : int);

    // Attributes
 public:
    std::vector< std::unordered_map<int, T> > items;

    // Associations
 public:

    /**
     * @element-type render_data_store
     */
    render_data_store *myrender_data_store;

    /**
     * @element-type mc_entity
     */
    mc_entity mymc_entity;

    /**
     * @element-type mc_chunk
     */
    mc_chunk mymc_chunk;

    /**
     * @element-type ivertex_buffer
     */
    ivertex_buffer myivertex_buffer;

    /**
     * @element-type itexture
     */
    itexture &myitexture;

    /**
     * @element-type ishader
     */
    ishader &myishader;

//end of class store
};

} /* End of namespace vulkan_mod */

#endif // VULKAN_MOD_STORE_H
