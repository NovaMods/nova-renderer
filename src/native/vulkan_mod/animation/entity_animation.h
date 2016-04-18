#ifndef VULKAN_MOD_ANIMATION_ENTITY_ANIMATION_H
#define VULKAN_MOD_ANIMATION_ENTITY_ANIMATION_H

// FILE: C:/Users/David/Documents/MC/force-src/src/main/native/vulkan_mod/animation/entity_animation.h

#include <vector>

#include "animation_frame.h"


namespace vulkan_mod {
namespace animation {


    /** 
     *  Represents an entity model.
     *  
     *  An entity model has a vertex buffer, a skeleton, and some animation data. The skeleton doesn't actually exist, I just say it does so my head doesn't get in a knot. The skeleton is actually just the animation data, and the animation data is a list of mat4s 
     */
class entity_animation {

    // Operations
 public:


    /** 
     *  Gets the animation start time from the time instance and marks this animation as running maybe?
     */
    virtual void start_animation();


    /** 
     *  Returns the pose at this time
     *  
     *  Binary searches through the animation frames to get the frame just before and just after the current time. Sends both those to the GPU. The animation shader will blend between those two poses based on the current time.
     *  
     *  The first thing in the returned pair is the previous pose, the second thing is the next pose.
     *  
     *  If the animation should loop, then when this method gets beyond the last frame it'll actually return the first frame, offset by the time of the animation. It does the offset by adding the animation's time to the begin_time variable. The begin_time is always added to the frames of returned animatino frames.
     */
    virtual std::pair<animation_frame, animation_frame> get_cur_pose();


    /** 
     *  Loads this animation from whatever file location I put animations in
     */
    virtual void load_from_resource(void  resource);

    virtual entity_animation(void  resource_location, animation_looping_behavior looping_behavior);


    /** 
     *  Tells this animation to no longer run
     */
    virtual void end_animation();

    // Attributes
 public:
    animation_looping_behavior looping_behavior;
    
    /** 
     *  If true, get_cur_pose returns that it says it does. If false, get_cur_pose returns the first frame with its time adjusted to be RIGHT NOW
     */
    bool is_running;

 protected:
    
    /** 
     *  The animation data.
     *  
     *  Each item in the top-level array is one animation frame, and each item in
     */
    std::vector< animation_frame > animation;
    
    /** 
     *  The time in seconds since the beginning of MC that this animation was started at
     *  
     *  Actually not entirely sure how to handle this. I think I need some sort of start time, though
     */
    double begin_time;

    // Associations
 public:

    /**
     * @element-type animation_frame
     */
    animation_frame myanimation_frame;

//end of class entity_animation
};

} /* End of namespace vulkan_mod::animation */
} /* End of namespace vulkan_mod */

#endif // VULKAN_MOD_ANIMATION_ENTITY_ANIMATION_H
