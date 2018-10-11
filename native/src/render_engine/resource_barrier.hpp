/*!
 * \brief Structs and enums and helper functions to use resource barriers
 *
 * \author ddubois
 * \date 01-Sep-18.
 */

#ifndef NOVA_RENDERER_RESOURCE_BARRIER_HPP
#define NOVA_RENDERER_RESOURCE_BARRIER_HPP

#include <memory>

namespace nova {
    struct iresource;

    /*!
     * \brief All the layouts that a resource may have
     *
     * This enum covers both images and buffers, and not all layouts are valid for all types of resources. The user of
     * this enum is supposed to know what's useful for what
     */
    enum class image_layout {
        RENDER_TARGET,
        PRESENT,
    };

    enum stage_flags { COLOR_ATTACHMENT_WRITE };

    /*!
     * \brief All the data needed for a resource barrier
     */
    struct resource_barrier_data {
        std::shared_ptr<iresource> resource_to_barrier;
        stage_flags source_access_flags;
        stage_flags dest_access_flags;
    };

    struct buffer_barrier_data : public resource_barrier_data {
        uint64_t offset;
        uint64_t range;
    };

    struct image_barrier_data : public resource_barrier_data {
        image_layout initial_layout;
        image_layout final_layout;
    };
}  // namespace nova

#endif  // NOVA_RENDERER_RESOURCE_BARRIER_HPP
