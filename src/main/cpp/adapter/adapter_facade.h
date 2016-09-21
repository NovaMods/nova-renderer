/*!
 * \brief
 *
 * \author ddubois 
 * \date 19-Sep-16.
 */

#ifndef RENDERER_ADAPTER_FACADE_H
#define RENDERER_ADAPTER_FACADE_H

#include <vector>
#include <unordered_map>
#include <string>
#include <functional>

#include "model/data_model.h"
#include "model/render_object.h"
#include "render_command/render_command.h"

namespace nova {
    namespace adapter {
        class adapter_facade {
            /*!
             * \brief Initializes the adapter facade to draw data from the given data_model
             *
             * \param model The data_model to adapt the data from
             */
            adapter_facade(model::data_model &model);

            /*!
             * \brief Determines if the user has loaded a new shaderpack
             *
             * \return True if there's a new shaderpack, false otherwise
             */
            bool has_new_shaderpack();

            /*!
             * \brief Gets all the data needed to configure Nova
             *
             * This data is a description of the shaderpack's render pipeline. It describes which shaders use which
             * buffers, and which shaders need new buffers. Nova can then build the framebuffers as needed, then swap
             * them out at runtime. This will be kinda slow, because switching framebuffers is the worst.
             *
             * The data also gives a number of filters which describe the kinds of geometry that each shader needs. The
             * filters can say things like "get all entities" or "get the transparent blocks in each chunk". The
             *
             * \return
             */
            config_varibles get_config_variables();

            /*!
             * \brief Sorts the geometry based on the provided filters
             *
             * The filters are grouped by string name. The filters are all applied to the
             *
             * \param filters The filters to apply to
             *
             * \return
             */
            std::unordered_map<std::string, std::vector<mesh>> sort_geometry(
                    std::unordered_map<std::string, std::function<bool(model::render_object)>> filters
            );

        private:
            model::data_model& model;

            bool cache_dirty;
        };
    }
}


#endif //RENDERER_ADAPTER_FACADE_H
