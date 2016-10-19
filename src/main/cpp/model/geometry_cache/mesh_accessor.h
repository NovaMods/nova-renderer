/*!
 * \brief
 *
 * \author ddubois 
 * \date 27-Sep-16.
 */

#ifndef RENDERER_GEOMETRY_CACHE_H
#define RENDERER_GEOMETRY_CACHE_H

#include <vector>
#include <mutex>
#include <functional>
#include <unordered_map>
#include "mc_interface/mc_objects.h"
#include "render_object.h"
#include "view/render_pass.h"

namespace nova {
    namespace model {
        /*!
         * \brief Provides access to the meshes that Nova will want to deal with
         *
         * The primary way it does this is by allowing the user to specify
         */
        class mesh_accessor {
        public:
            std::vector<render_object*> get_meshes_for_filter(std::function<bool(const render_object&)>& filter);

            void add_render_object(const render_object& new_obj);

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
        private:
            std::mutex render_objects_lock;
            std::vector<render_object> renderable_objects;
        };
    }
}

#endif //RENDERER_GEOMETRY_CACHE_H
