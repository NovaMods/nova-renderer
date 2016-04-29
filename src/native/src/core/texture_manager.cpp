/*!
 * \author David
 * \date 29-Apr-16.
 */

#include "texture_manager.h"

texture_manager::texture_manager() {

}

texture_manager::~texture_manager() {
    // gotta free up all the OpenGL textures
    // The driver probably does that for me, but I ain't about to let no stinkin' driver boss me around!
    reset();
}



