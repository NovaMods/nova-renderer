#pragma once

#include "nova_renderer/camera.hpp"
#include "nova_renderer/constants.hpp"
#include "nova_renderer/rhi/render_device.hpp"
#include "rx/core/set.h"
#include "rx/core/queue.h"

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

        CameraUboData& operator[](uint32_t idx);

        void upload_to_device(uint32_t frame_idx);

        [[nodiscard]] uint32_t get_next_free_camera_index();

        void free_camera_index(uint32_t idx);

    private:
        rx::array<rhi::RhiBuffer* [NUM_IN_FLIGHT_FRAMES]> per_frame_buffers;
        rhi::RenderDevice& device;

        CameraUboData data[MAX_NUM_CAMERAS];
        rx::queue<uint32_t> free_camera_indices;
    };
} // namespace nova::renderer
