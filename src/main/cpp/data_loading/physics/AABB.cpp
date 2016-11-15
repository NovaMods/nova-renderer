/*!
 * \brief
 *
 * \author ddubois 
 * \date 09-Oct-16.
 */

#include "AABB.h"

namespace nova {
    namespace model {
        void AABB::set_extents(glm::vec3& new_extents) {
            extents = new_extents;
        }

        void AABB::translate(glm::vec3 &delta) {
            center += delta;
        }

        void AABB::set_position(glm::vec3 &position) {
            center = position;
        }
    }
}
