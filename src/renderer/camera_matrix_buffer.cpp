#include "nova_renderer/camera_matrix_buffer.hpp"

namespace nova::renderer {
    CameraMatrixBuffer::CameraMatrixBuffer(rhi::RenderDevice& device, rx::memory::allocator& internal_allocator)
        : free_camera_indices{&internal_allocator} {
        rhi::RhiBufferCreateInfo create_info;
        create_info.size = sizeof(CameraUboData) * MAX_NUM_CAMERAS;
        create_info.buffer_usage = rhi::BufferUsage::UniformBuffer;

        for(uint32_t i = 0; i < NUM_IN_FLIGHT_FRAMES; i++) {
            create_info.name = rx::string::format("CameraBuffer%u", i);

            per_frame_buffers[i] = device.create_buffer(create_info. internal_allocator);
        }
    }

} // namespace nova::renderer
