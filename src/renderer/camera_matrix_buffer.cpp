#include "nova_renderer/camera_matrix_buffer.hpp"

namespace nova::renderer {
    CameraMatrixBuffer::CameraMatrixBuffer(rhi::RenderDevice& device, rx::memory::allocator& internal_allocator)
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

    CameraUboData& CameraMatrixBuffer::operator[](const uint32_t idx) { return data[idx]; }

    void CameraMatrixBuffer::upload_to_device(const uint32_t frame_idx) {
        device.write_data_to_buffer(data, sizeof(data), 0, per_frame_buffers[frame_idx]);
    }

    uint32_t CameraMatrixBuffer::get_next_free_camera_index() { return free_camera_indices.pop(); }

    void CameraMatrixBuffer::free_camera_index(const uint32_t idx) { free_camera_indices.emplace(idx); }
} // namespace nova::renderer
