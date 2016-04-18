#ifndef VULKAN_MOD_GRAPHICS_OBJECT_FACTORY_H
#define VULKAN_MOD_GRAPHICS_OBJECT_FACTORY_H

// FILE: C:/Users/David/Documents/MC/force-src/src/main/native/vulkan_mod/graphics_object_factory.h

#include "ivertex_buffer.h"


namespace vulkan_mod {

class graphics_object_factory {

    // Operations
 public:


    /** 
     *  Creates a new texture object
     */
    virtual void create_texture()  = 0;


    /** 
     *  Creates and returns a new vertex buffer
     */
    virtual ivertex_buffer create_vertex_buffer()  = 0;

    virtual void create_shader()  = 0;

    // Associations

//end of class graphics_object_factory
};

} /* End of namespace vulkan_mod */

#endif // VULKAN_MOD_GRAPHICS_OBJECT_FACTORY_H
