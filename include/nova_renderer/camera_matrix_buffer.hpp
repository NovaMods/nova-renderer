#pragma once

#include "nova_renderer/camera.hpp"
#include "nova_renderer/constants.hpp"
#include "nova_renderer/rhi/render_device.hpp"

#include "rx/core/queue.h"
#include "rx/core/set.h"

namespace nova::renderer {
    constexpr uint32_t MAX_NUM_CAMERAS = 65536;

    /*!
     * \brief Array of data which is unique for each frame of execution
     */
    template <typename ElementType>
    class PerFrameDeviceArray {
    public:
        /*!
         * \brief Initializes a CameraMatrixBuffer, creating GPU resources with the provided render device
         *
         * \param device The device to create the camera buffers on
         * \param internal_allocator The allocator to use for internal allocations
         */
        explicit PerFrameDeviceArray(rhi::RenderDevice& device, rx::memory::allocator& internal_allocator);

        ElementType& operator[](uint32_t idx);

        void upload_to_device(uint32_t frame_idx);

        [[nodiscard]] uint32_t get_next_free_camera_index();

        void free_camera_index(uint32_t idx);

    private:
        rx::array<rhi::RhiBuffer* [NUM_IN_FLIGHT_FRAMES]> per_frame_buffers;
        rhi::RenderDevice& device;

        ElementType data[MAX_NUM_CAMERAS];
        rx::queue<uint32_t> free_camera_indices;
    };

    template <typename ElementType>
    PerFrameDeviceArray<ElementType>::PerFrameDeviceArray(rhi::RenderDevice& device, rx::memory::allocator& internal_allocator)
        : device{device}, free_camera_indices{&internal_allocator} {
        rhi::RhiBufferCreateInfo create_info;
        create_info.size = sizeof(CameraUboData) * MAX_NUM_CAMERAS;
        create_info.buffer_usage = rhi::BufferUsage::UniformBuffer;

        for(uint32_t i = 0; i < NUM_IN_FLIGHT_FRAMES; i++) {
            create_info.name = rx::string::format("CameraBuffer%u", i);

            per_frame_buffers[i] = device.create_buffer(create_info, internal_allocator);
        }

        // All camera indices are free at program startup
        for(uint32_t i = 0; i < MAX_NUM_CAMERAS; i++) {
            free_camera_indices.emplace(i);
        }
    }

    template <typename ElementType>
    ElementType& PerFrameDeviceArray<ElementType>::operator[](const uint32_t idx) {
        return data[idx];
    }

    template <typename ElementType>
    void PerFrameDeviceArray<ElementType>::upload_to_device(const uint32_t frame_idx) {
        device.write_data_to_buffer(data, sizeof(data), 0, per_frame_buffers[frame_idx]);
    }

    template <typename ElementType>
    uint32_t PerFrameDeviceArray<ElementType>::get_next_free_camera_index() {
        return free_camera_indices.pop();
    }

    template <typename ElementType>
    void PerFrameDeviceArray<ElementType>::free_camera_index(const uint32_t idx) {
        free_camera_indices.emplace(idx);
    }
} // namespace nova::renderer
