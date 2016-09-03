#ifndef VULKAN_MOD_ISHADER_H
#define VULKAN_MOD_ISHADER_H

// FILE: C:/Users/David/Documents/MC/force-src/src/main/native/vulkan_mod/ishader.h

#include "igraphics_object.h"


namespace vulkan_mod {


    /** 
     *  Represents a shader in an API-agnostic way
     */
class ishader : virtual public igraphics_object {

    // Operations
 public:


    /** 
     *  Loads a shader from the specified file
     *  
     *  Location should be the path to the shader (from the shaderpacks folder)
     */
    virtual void load_from_file(std::string file_location)  = 0;


    /** 
     *  Loads a GLSL shader from the specified location
     */
    virtual void load_glsl_from_file(std::string file_location)  = 0;

    // Associations

//end of class ishader
};

} /* End of namespace vulkan_mod */

#endif // VULKAN_MOD_ISHADER_H
