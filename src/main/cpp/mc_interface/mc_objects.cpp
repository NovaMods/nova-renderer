/*!
 * \brief
 *
 * \author ddubois 
 * \date 02-Mar-17.
 */

#include "mc_objects.h"

#include <sstream>
#include <json.hpp>

bool mc_block::is_emissive() const {
    return light_value > 0;
}

bool mc_block::is_transparent() const {
    return is_opaque == 0;
}
