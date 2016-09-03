#ifndef VULKAN_MOD_ITEXTURE_H
#define VULKAN_MOD_ITEXTURE_H

// FILE: C:/Users/David/Documents/MC/force-src/src/main/native/vulkan_mod/itexture.h

#include "igraphics_object.h"


namespace vulkan_mod {

class itexture : virtual public igraphics_object {

    // Operations
 public:

    virtual void set_data(std::vector<float> data, int height, int width, int channels);

    // Associations

//end of class itexture
};

} /* End of namespace vulkan_mod */

#endif // VULKAN_MOD_ITEXTURE_H
