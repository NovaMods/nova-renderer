#ifndef VULKAN_MOD_IVERTEX_BUFFER_H
#define VULKAN_MOD_IVERTEX_BUFFER_H

// FILE: C:/Users/David/Documents/MC/force-src/src/main/native/vulkan_mod/ivertex_buffer.h

#include "igraphics_object.h"

namespace vulkan_mod {
class mc_chunk;
} /* End of namespace vulkan_mod */

namespace vulkan_mod {


    /** 
     *  Represents a vertex buffer (of some sort) in a device-agnostic way
     */
class ivertex_buffer : virtual public igraphics_object {

    // Operations
 public:

    virtual void set_data(std::vector<float> data, vertex_format format)  = 0;

    // Associations
 public:


    /**
     * @element-type mc_chunk
     */
    mc_chunk *mymc_chunk;

//end of class ivertex_buffer
};

} /* End of namespace vulkan_mod */

#endif // VULKAN_MOD_IVERTEX_BUFFER_H
