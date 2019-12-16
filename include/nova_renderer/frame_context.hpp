#pragma once

#include <cstddef>

#include "nova_renderer/rhi/forward_decls.hpp"

namespace nova::renderer {
    class NovaRenderer;

    /*!
     * \brief All the per-frame data that Nova itself cares about
     */
    struct FrameContext {
        /*!
         * \brief Pointer to the NovaRenderer instance that launched this frame
         */
        NovaRenderer* nova;

        /*!
         * \brief The number of frames that were started before this frame
         */
        size_t frame_count;

        /*!
         * \brief Swapchain image that this frame renders to
         */
        rhi::Image* swapchain_image;

        /*!
         * \brief Swapchain framebuffer that this frame renders to
         */
        rhi::Framebuffer* swapchain_framebuffer;

        size_t cur_model_matrix_index = 0;
    };
}
