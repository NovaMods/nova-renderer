#pragma once

#include "nova_renderer/camera.hpp"
#include "nova_renderer/constants.hpp"
#include "nova_renderer/rhi/render_device.hpp"

namespace nova::renderer {
    constexpr uint32_t MAX_NUM_CAMERAS = 65536;

    /*!
     * \brief Buffer that holds all the camera matrices. Contains logic to allocate/deallocate space in the buffer for each camera and to
     * upload the camera matrices to the GPU
     */
    class CameraMatrixBuffer {
    public:
        /*!
         * \brief Initializes a CameraMatrixBuffer, creating GPU resources with the provided render device
         *
         * \param device The device to create the camera buffers on
         * \param internal_allocator The allocator to use for internal allocations
         */
        explicit CameraMatrixBuffer(rhi::RenderDevice& device, rx::memory::allocator& internal_allocator);

    private:
        rx::array<rhi::RhiBuffer* [NUM_IN_FLIGHT_FRAMES]> per_frame_buffers;

        CameraUboData data[MAX_NUM_CAMERAS];
        rx::vector<uint32_t> free_camera_indices;
    };
} // namespace nova::renderer
