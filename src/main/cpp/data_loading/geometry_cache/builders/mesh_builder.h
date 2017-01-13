/*!
 * \brief
 *
 * \author ddubois 
 * \date 14-Nov-16.
 */

#ifndef RENDERER_MESH_BUILDER_H
#define RENDERER_MESH_BUILDER_H

#include <unordered_map>
#include <mutex>
#include "../../../mc_interface/mc_objects.h"
#include "../mesh_definition.h"

namespace nova {
    /*!
     * \brief Builds some geometry for the GUI from the provided GUI data structure
     */
    mesh_definition build_geometry(mc_gui_screen &screen);

    /*!
     * \brief Builds some geometry for the chunk described by the given data structure
     */
    mesh_definition build_geometry(mc_chunk &chunk);

    /*!
     * \brief Builds geometry for the provided entity definition
     */
    mesh_definition build_geometry(mc_entity &entity);


}


#endif //RENDERER_MESH_BUILDER_H
