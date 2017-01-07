/*!
 * \brief
 *
 * \author ddubois 
 * \date 09-Oct-16.
 */

#ifndef RENDERER_AABB_H
#define RENDERER_AABB_H

#include <glm/glm.hpp>
#include "utils/export.h"

namespace nova {
    /*!
     * \brief Represents an axis-aligned bounding box.
     *
     * Contains methods to help it be a productive member of society
     */
    class NOVA_API AABB {
    public:
        void set_extents(glm::vec3 &new_extents);

        void translate(glm::vec3 &delta);

        void set_position(glm::vec3 &position);

        // TODO: More methods
    private:
        glm::vec3 center;   //!< The local space center of this AABB
        glm::vec3 extents;  //!< How far in each direction this AABB reaches
    };
}


#endif //RENDERER_AABB_H
