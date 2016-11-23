/*!
 * \brief
 *
 * \author ddubois 
 * \date 14-Nov-16.
 */

#ifndef RENDERER_MESH_BUILDER_H
#define RENDERER_MESH_BUILDER_H

#include <mc_interface/mc_objects.h>
#include <data_loading/geometry_cache/mesh_definition.h>
#include <unordered_map>
#include <mutex>
#include <data_loading/loaded_resource.h>

namespace nova {
    namespace model {
        /*!
         * \brief Builds meshes from various mesh definitions, and holds those meshes until the render thread can scoop
         * them up
         */
        class mesh_builder {
        public:

            /*!
             * \brief Builds some geometry for the GUI from the provided GUI data structure
             */
            void build_geometry(mc_gui_screen& screen);

            /*!
             * \brief Builds some geometry for the chunk described by the given data structure
             */
            void build_geometry(mc_chunk& chunk);

            /*!
             * \brief Builds geometry for the provided entity definition
             */
            void build_geometry(mc_entity& entity);

            loaded_resource<mesh_definition>& get_gui_mesh();

        private:
            loaded_resource<mesh_definition> gui_mesh;
        };
    }
}


#endif //RENDERER_MESH_BUILDER_H
