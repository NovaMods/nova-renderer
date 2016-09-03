#ifndef VULKAN_MOD_RENDER_DATA_STORE_H
#define VULKAN_MOD_RENDER_DATA_STORE_H

// FILE: C:/Users/David/Documents/MC/force-src/src/main/native/vulkan_mod/render_data_store.h

#include "graphics_object_factory.h"
#include "mc_chunk.h"
#include "mc_entity.h"
#include "store.h"


namespace vulkan_mod {

class render_data_store {

    // Operations
 public:


    /** 
     *  Updates the given chunk by finding the chunk with the given id (or creating it if it doesn't exist), then copying the data from the given mc_chunk into the old one
     */
    virtual void update_chunk(mc_chunk new_chunk);


    /** 
     *  Updates the given entity by finding the entity with the given ID (or creating it if it doesn't already exist) and copying the new entity into the old one
     */
    virtual void update_entity(mc_entity new_entity);


    /** 
     *  Returns all chunks. Simple as that.
     */
    virtual store get_all_chunks();


    /** 
     *  Returns all entities
     */
    virtual store get_all_entities();

    // Attributes

 protected:
    store texture_store;
    store shader_store;
    store entity_model_store;
    store chunk_store;
    store entity_store;
    
    /** 
     *  The factory to be used when generating GPU objects
     */
    graphics_object_factory &graphics_factory;

    // Associations
 public:

    /**
     * @element-type store
     */
    store mystore;

    /**
     * @element-type graphics_object_factory
     */
    graphics_object_factory &mygraphics_object_factory;

//end of class render_data_store
};

} /* End of namespace vulkan_mod */

#endif // VULKAN_MOD_RENDER_DATA_STORE_H
