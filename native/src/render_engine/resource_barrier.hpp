/*!
 * \brief Structs and enums and helper functions to use resource barriers
 *
 * \author ddubois 
 * \date 01-Sep-18.
 */

#ifndef NOVA_RENDERER_RESOURCE_BARRIER_HPP
#define NOVA_RENDERER_RESOURCE_BARRIER_HPP

#include "../util/smart_enum.hpp"

namespace nova {
    class resource;

    /*!
     * \brief All the layouts that a resoruce may have
     *
     * This enum covers both images and buffers, and not all layouts are valid for all types of resources. The user of
     * this enum is supposed to know what's useful for what
     */
    SMART_ENUM(resource_layout,
               /*!
                * \brief The resource is going to be used as a render target
                *
                * Only valid for images
                */
               RENDER_TARGET,

               /*!
                * \brief The resource will be presented to the user's screen
                *
                * Only valid for images
                */
               PRESENT,

    )

    /*!
     * \brief All the data needed for a resource barrier
     */
    struct resource_barrier_data {
        resource* resource_to_barrier;
        resource_layout initial_layout;
        resource_layout final_layout;
    };
}

#endif //NOVA_RENDERER_RESOURCE_BARRIER_HPP
