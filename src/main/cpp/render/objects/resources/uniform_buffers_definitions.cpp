/*!
 * \brief Defines the functions defclared in uniform_buffer_definitions.h. This is mostly printing operators because
 * debugging is hard
 *
 * \author ddubois 
 * \date 22-Jan-17.
 */

#include "uniform_buffer_definitions.h"

namespace nova {
    el::base::Writer &operator<<(el::base::Writer &out, const glm::mat4 &mat) {
        out << "{ " << mat[0].x << " " << mat[0].y << " " << mat[0].z << " " << mat[0].z << "\n"
            << " " << mat[1].x << " " << mat[1].y << " " << mat[1].z << " " << mat[1].z << "\n"
            << " " << mat[2].x << " " << mat[2].y << " " << mat[2].z << " " << mat[2].z << "\n"
            << " " << mat[3].x << " " << mat[3].y << " " << mat[3].z << " " << mat[3].z << " }";

		return out;
    }
}
