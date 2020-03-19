#pragma once
#include <rx/core/map.h>
#include <rx/core/string.h>

#include "nova_renderer/rhi/pipeline_create_info.hpp"

namespace nova::renderer {
    struct ResourceBindingLocation {
        uint32_t set;
        uint32_t binding;
    };

    /*!
     * \brief Retrieves the binding locations of all bespoke images that the graphics pipeline uses
     *
     * Bespoke images are any images that aren't part of the textures array. This typically only includes render targets and other images
     * that shaders write to
     */
    rx::map<rx::string, ResourceBindingLocation> get_bespoke_image_binding_locations(const RhiGraphicsPipelineState& pipeline_state,
                                                                                     rx::memory::allocator& allocator);
} // namespace nova::renderer
