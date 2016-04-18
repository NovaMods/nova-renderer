#ifndef VULKAN_MOD_RENDERER_H
#define VULKAN_MOD_RENDERER_H

// FILE: C:/Users/David/Documents/MC/force-src/src/main/native/vulkan_mod/renderer.h

#include "store.h"


namespace vulkan_mod {


    /** 
     *  Class that handles actually drawing things onscreen. May or may not change to a namespace
     */
class renderer {

    // Operations
 public:


    /** 
     *  Renders the current scene
     */
    virtual void render_scene();


    /** 
     *  Renders all the entities by (in OpenGL, at least) instancing the entity meshes
     */
    virtual void render_all_entites(store entities)  = 0;


    /** 
     *  Renders all the chunks. Assuming chunks have their own VBOs, I can just render each chunk's VBO. It'll be gross and I'll hate it but #YOLO
     */
    virtual void render_all_chunks(store chunks)  = 0;

//end of class renderer
};

} /* End of namespace vulkan_mod */

#endif // VULKAN_MOD_RENDERER_H
