/*!
 * \brief
 *
 * \author ddubois 
 * \date 09-Oct-16.
 */

#ifndef RENDERER_AABB_H
#define RENDERER_AABB_H

#include <glm/glm.hpp>

namespace nova {
    /*!
     * \brief Represents an axis-aligned bounding box.
     *
     * Contains methods to help it be a productive member of society
     */
    struct aabb {
        glm::vec3 center;   //!< The local space center of this AABB
        glm::vec3 extents;  //!< How far in each direction this AABB reaches

        void translate(glm::vec3 &delta);
    };
}


#endif //RENDERER_AABB_H
