/*!
 * \brief
 *
 * \author ddubois 
 * \date 06-Oct-16.
 */

#ifndef RENDERER_GEOMETRY_BUILDER_H
#define RENDERER_GEOMETRY_BUILDER_H

#include "mc_interface/mc_objects.h"

namespace nova {
    namespace view {
        /*!
         * \brief Builds the geometry from the raw definitions that Minecraft provides
         *
         * The idea is that, when data is sent from Minecraft it'll go into a queue here. The data uploading thread will
         * (eventually) come along and build the actual geometry, then upload it to the GPU, saving the references to
         * the mesh accessor
         */
        class geometry_builder {
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
        };
    }
}

#endif //RENDERER_GEOMETRY_BUILDER_H
