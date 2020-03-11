#pragma once

#include <minitrace.h>
#include <rx/core/vector.h>

#include "nova_renderer/camera.hpp"
#include "nova_renderer/constants.hpp"
#include "nova_renderer/rhi/render_device.hpp"

namespace nova::renderer {
    /*!
     * \brief Array of data which is unique for each frame of execution
     */
    template <typename ElementType>
    class PerFrameDeviceArray {
    public:
        /*!
         * \brief Initializes a PerFrameDeviceArray, creating GPU resources with the provided render device
         *
         * \param num_elements The number of elements in the array
         * \param num_in_flight_frames NThe number of in-flight frames that we'll support
         * \param device The device to create the buffers on
         * \param internal_allocator The allocator to use for internal allocations
         */
        explicit PerFrameDeviceArray(size_t num_elements,
                                     uint32_t num_in_flight_frames,
                                     rhi::RenderDevice& device,
                                     rx::memory::allocator& internal_allocator);

        ~PerFrameDeviceArray() = default;

        ElementType& operator[](uint32_t idx);

        void upload_to_device(uint32_t frame_idx);

        [[nodiscard]] uint32_t get_next_free_slot();

        void free_slot(uint32_t idx);

        [[nodiscard]] size_t size() const;

    private:
        rx::memory::allocator& internal_allocator;

        rx::vector<rhi::RhiBuffer*> per_frame_buffers;
        rhi::RenderDevice& device;

        ElementType* data;
        size_t num_elements;

        rx::vector<uint32_t> free_indices;
    };

    template <typename ElementType>
    PerFrameDeviceArray<ElementType>::PerFrameDeviceArray(const size_t num_elements,
                                                          const uint32_t num_in_flight_frames,
                                                          rhi::RenderDevice& device,
                                                          rx::memory::allocator& internal_allocator)
        : internal_allocator{internal_allocator},
          device{device},
          data{reinterpret_cast<ElementType*>(internal_allocator.allocate(num_elements * sizeof(ElementType)))},
          num_elements{num_elements},
          free_indices{&internal_allocator} {
        rhi::RhiBufferCreateInfo create_info;
        create_info.size = sizeof(CameraUboData) * MAX_NUM_CAMERAS;
        create_info.buffer_usage = rhi::BufferUsage::UniformBuffer;

        per_frame_buffers.reserve(num_in_flight_frames);
        for(uint32_t i = 0; i < num_in_flight_frames; i++) {
            create_info.name = rx::string::format("CameraBuffer%u", i);

            per_frame_buffers.emplace_back(device.create_buffer(create_info, internal_allocator));
        }

        // All camera indices are free at program startup
        for(uint32_t i = 0; i < MAX_NUM_CAMERAS; i++) {
            free_indices.emplace_back(i);
        }
    }

    template <typename ElementType>
    ElementType& PerFrameDeviceArray<ElementType>::operator[](const uint32_t idx) {
        return data[idx];
    }

    template <typename ElementType>
    void PerFrameDeviceArray<ElementType>::upload_to_device(const uint32_t frame_idx) {
        MTR_SCOPE("PerFrameDeviceArray", "upload_to_device");
        device.write_data_to_buffer(data, sizeof(data), 0, per_frame_buffers[frame_idx]);
    }

    template <typename ElementType>
    uint32_t PerFrameDeviceArray<ElementType>::get_next_free_slot() {
        const auto val = free_indices.last();

        const auto erase_idx = free_indices.size() - 1;
        free_indices.erase(erase_idx, erase_idx);

        return val;
    }

    template <typename ElementType>
    void PerFrameDeviceArray<ElementType>::free_slot(const uint32_t idx) {
        free_indices.emplace_back(idx);
    }

    template <typename ElementType>
    size_t PerFrameDeviceArray<ElementType>::size() const {
        return num_elements;
    }
} // namespace nova::renderer
