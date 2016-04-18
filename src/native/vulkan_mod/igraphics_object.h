#ifndef VULKAN_MOD_IGRAPHICS_OBJECT_H
#define VULKAN_MOD_IGRAPHICS_OBJECT_H

// FILE: C:/Users/David/Documents/MC/force-src/src/main/native/vulkan_mod/igraphics_object.h


namespace vulkan_mod {


    /** 
     *  Interface for graphics objects providing common methods to do things like bind, create, destroy, and whatnot
     */
class igraphics_object {

    // Operations
 public:


    /** 
     *  Creates a new thing, allocating it on the GPU
     */
    virtual void create()  = 0;


    /** 
     *  Removes this thing from VRAM
     */
    virtual void destroy()  = 0;


    /** 
     *  Tells the graphics driver to bind this object to the given identifier
     */
    virtual void bind(int location)  = 0;


    /** 
     *  Tells the graphics driver to no longer use this object. 
     */
    virtual void unbind()  = 0;


    /** 
     *  Checks if this object is currently bound, returning true if it is
     */
    virtual bool is_bound()  = 0;

public:
    // virtual destructor for interface 
    virtual ~igraphics_object() { }

//end of class igraphics_object
};

} /* End of namespace vulkan_mod */

#endif // VULKAN_MOD_IGRAPHICS_OBJECT_H
