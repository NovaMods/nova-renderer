/*!
 * \brief
 *
 * \author ddubois 
 * \date 09-Oct-16.
 */

#include "aabb.h"

namespace nova {
    void aabb::translate(glm::vec3 &delta) {
        center += delta;
    }
}