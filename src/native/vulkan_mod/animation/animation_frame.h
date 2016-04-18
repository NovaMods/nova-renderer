#ifndef VULKAN_MOD_ANIMATION_ANIMATION_FRAME_H
#define VULKAN_MOD_ANIMATION_ANIMATION_FRAME_H

// FILE: C:/Users/David/Documents/MC/force-src/src/main/native/vulkan_mod/animation/animation_frame.h

#include <vector>



namespace vulkan_mod {
namespace animation {

class animation_frame {

    // Attributes
 public:
    
    /** 
     *  The state of all the bones at this animation frame
     */
    std::vector< glm::mat4 > pose;
    
    /** 
     *  The time in seconds since the beginning of the animation that this pose occurs at
     */
    double sample_time;

    // Associations

//end of class animation_frame
};

} /* End of namespace vulkan_mod::animation */
} /* End of namespace vulkan_mod */

#endif // VULKAN_MOD_ANIMATION_ANIMATION_FRAME_H
