#ifndef VULKAN_MOD_TIME_H
#define VULKAN_MOD_TIME_H

// FILE: C:/Users/David/Documents/MC/force-src/src/main/native/vulkan_mod/time.h

#include "time.h"


namespace vulkan_mod {


    /** 
     *  Represents time
     */
class time {

    // Operations
 public:


    /** 
     *  Returns the time in seconds since Minecraft was started
     */
    virtual double get_cur_time();


    /** 
     *  Returns the time in seconds that the last frame took to render
     */
    virtual double get_delta_time();


    /** 
     *  Reads the CPU clock to know when the frame started rendering.
     *  
     *  Also updates delta time and current time
     */
    virtual void start_frame();

    // Attributes
 public:
    
    /** 
     *  Static. SIngleton.
     */
    time the_time;

 protected:
    
    /** 
     *  The time in seconds since Minecraft was started
     */
    double cur_time;
    
    /** 
     *  The time in seconds that the last frame took to render
     */
    double delta_time;

//end of class time
};

} /* End of namespace vulkan_mod */

#endif // VULKAN_MOD_TIME_H
