#pragma once

#include <stddef.h>

#include <rx/core/memory/allocator.h>

#include "nova_renderer/rhi/forward_decls.hpp"
#include "nova_renderer/resource_loader.hpp"

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
         * \brief Index of the frame currently being rendered
         */
        size_t frame_idx;

        /*!
         * \brief Swapchain image that this frame renders to
         */
        rhi::RhiImage* swapchain_image;

        /*!
         * \brief Swapchain framebuffer that this frame renders to
         */
        rhi::RhiFramebuffer* swapchain_framebuffer;

        /*!
         * \brief Buffer with all the camera matrices to use when rendering this frame
         */
        rhi::RhiBuffer* camera_matrix_buffer;

        size_t cur_model_matrix_index = 0;

        rx::memory::allocator* allocator = nullptr;

        BufferResourceAccessor material_buffer;
    };
} // namespace nova::renderer
