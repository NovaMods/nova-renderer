#include "nova_renderer/procedural_mesh.hpp"

#include <rx/core/log.h>

#include "nova_renderer/rhi/render_device.hpp"

#include "../util/memory_utils.hpp"

using namespace nova::mem;

using rx::utility::move;

namespace nova::renderer {
    RX_LOG("ProceduralMesh", logger);

    using namespace rhi;

    ProceduralMesh::ProceduralMesh(const uint64_t vertex_buffer_size_in,
                                   const uint64_t index_buffer_size_in,
                                   const uint32_t num_in_flight_frames,
                                   RenderDevice* device_in,
                                   const rx::string& name_in)
        : device(device_in), name(name_in), allocator {
        &rx::memory::system_allocator::instance()
    }
#ifdef NOVA_DEBUG
    , vertex_buffer_size(vertex_buffer_size_in),
        index_buffer_size(index_buffer_size_in)
#endif
    {

        vertex_buffers.reserve(num_in_flight_frames);
        index_buffers.reserve(num_in_flight_frames);
        for(uint32_t i = 0; i < num_in_flight_frames; i++) {
            vertex_buffers.push_back(
                device_in->create_buffer({rx::string::format("%sVertices%d", name_in, i), vertex_buffer_size_in, BufferUsage::VertexBuffer},
                                         *allocator));
            index_buffers.push_back(
                device_in->create_buffer({rx::string::format("%sIndices%d", name_in, i), index_buffer_size_in, BufferUsage::IndexBuffer},
                                         *allocator));
        }

        cached_vertex_buffer = device_in->create_buffer({rx::string::format("%sStagingVertices", name_in),
                                                         vertex_buffer_size_in,
                                                         BufferUsage::StagingBuffer},
                                                        *allocator);
        cached_index_buffer = device_in->create_buffer({rx::string::format("%sStagingIndices", name_in),
                                                        index_buffer_size_in,
                                                        BufferUsage::StagingBuffer},
                                                       *allocator);
    }

    ProceduralMesh::ProceduralMesh(ProceduralMesh&& old) noexcept
        : device{old.device},
          vertex_buffers{move(old.vertex_buffers)},
          index_buffers{move(old.index_buffers)},
          cached_vertex_buffer{move(old.cached_vertex_buffer)},
          cached_index_buffer{move(old.cached_index_buffer)},
          num_vertex_bytes_to_upload{old.num_vertex_bytes_to_upload},
          num_index_bytes_to_upload{old.num_index_bytes_to_upload},
          allocator {
        old.allocator
    }
#ifdef NOVA_DEBUG
    , vertex_buffer_size{old.vertex_buffer_size}, index_buffer_size { old.index_buffer_size }
#endif
    { old.device = nullptr; }

    ProceduralMesh& ProceduralMesh::operator=(ProceduralMesh&& old) noexcept {
        device = old.device;
        vertex_buffers = move(old.vertex_buffers);
        index_buffers = move(old.index_buffers);
        cached_vertex_buffer = move(old.cached_vertex_buffer);
        cached_index_buffer = move(old.cached_index_buffer);
        num_vertex_bytes_to_upload = old.num_vertex_bytes_to_upload;
        num_index_bytes_to_upload = old.num_index_bytes_to_upload;
        allocator = old.allocator;

#ifdef NOVA_DEBUG
        vertex_buffer_size = old.vertex_buffer_size;
        index_buffer_size = old.index_buffer_size;
#endif

        old.device = nullptr;

        return *this;
    }

    void ProceduralMesh::set_vertex_data(const void* data, const uint64_t size) {
#ifdef NOVA_DEBUG
        if(size > vertex_buffer_size) {
            logger->error(
                "Cannot upload vertex data. There's only space for %uz bytes, you tried to upload %uz. Truncating vertex data to fit",
                vertex_buffer_size,
                size);

            device->write_data_to_buffer(data, vertex_buffer_size, *cached_vertex_buffer);
            num_vertex_bytes_to_upload = vertex_buffer_size;

        } else {
#endif

            device->write_data_to_buffer(data, size, *cached_vertex_buffer);
            num_vertex_bytes_to_upload = size;

#ifdef NOVA_DEBUG
        }
#endif
    }

    void ProceduralMesh::set_index_data(const void* data, const uint64_t size) {
#ifdef NOVA_DEBUG
        if(size > index_buffer_size) {
            logger->error(
                "Cannot upload index data. There's only space for %uz bytes, you tried to upload %uz. Truncating vertex data to fit",
                index_buffer_size,
                size);

            device->write_data_to_buffer(data, index_buffer_size, *cached_index_buffer);
            num_index_bytes_to_upload = index_buffer_size;

        } else {
#endif
            device->write_data_to_buffer(data, size, *cached_index_buffer);
            num_index_bytes_to_upload = size;

#ifdef NOVA_DEBUG
        }
#endif
    }

    void ProceduralMesh::record_commands_to_upload_data(RhiRenderCommandList* cmds, const uint8_t frame_idx) const {
        const bool should_upload_vertex_buffer = num_vertex_bytes_to_upload > 0;
        const bool should_upload_index_buffer = num_index_bytes_to_upload > 0;

        auto* cur_vertex_buffer = vertex_buffers[frame_idx].get();
        auto* cur_index_buffer = index_buffers[frame_idx].get();

        rx::vector<RhiResourceBarrier> barriers_before_upload;

        if(should_upload_vertex_buffer) {
            RhiResourceBarrier barrier_before_vertex_upload = {};
            barrier_before_vertex_upload.resource_to_barrier = cur_vertex_buffer;
            barrier_before_vertex_upload.access_before_barrier = ResourceAccess::VertexAttributeRead;
            barrier_before_vertex_upload.access_after_barrier = ResourceAccess::MemoryWrite;
            barrier_before_vertex_upload.old_state = ResourceState::VertexBuffer;
            barrier_before_vertex_upload.new_state = ResourceState::CopyDestination;
            barrier_before_vertex_upload.source_queue = QueueType::Graphics;
            barrier_before_vertex_upload.destination_queue = QueueType::Graphics;
            barrier_before_vertex_upload.buffer_memory_barrier.offset = 0;
            barrier_before_vertex_upload.buffer_memory_barrier.size = num_vertex_bytes_to_upload;

            barriers_before_upload.push_back(barrier_before_vertex_upload);
        }

        if(should_upload_index_buffer) {
            RhiResourceBarrier barrier_before_index_upload = {};
            barrier_before_index_upload.resource_to_barrier = cur_index_buffer;
            barrier_before_index_upload.access_before_barrier = ResourceAccess::IndexRead;
            barrier_before_index_upload.access_after_barrier = ResourceAccess::MemoryWrite;
            barrier_before_index_upload.old_state = ResourceState::IndexBuffer;
            barrier_before_index_upload.new_state = ResourceState::CopyDestination;
            barrier_before_index_upload.source_queue = QueueType::Graphics;
            barrier_before_index_upload.destination_queue = QueueType::Graphics;
            barrier_before_index_upload.buffer_memory_barrier.offset = 0;
            barrier_before_index_upload.buffer_memory_barrier.size = num_index_bytes_to_upload;

            barriers_before_upload.push_back(barrier_before_index_upload);
        }

        if(barriers_before_upload.is_empty()) {
            return;
        }

        cmds->resource_barriers(PipelineStage::VertexInput, PipelineStage::Transfer, barriers_before_upload);

        if(should_upload_vertex_buffer) {
            cmds->copy_buffer(*cur_vertex_buffer, 0, *cached_vertex_buffer, 0, num_vertex_bytes_to_upload);
        }

        if(should_upload_index_buffer) {
            cmds->copy_buffer(*cur_index_buffer, 0, *cached_index_buffer, 0, num_index_bytes_to_upload);
        }

        rx::vector<RhiResourceBarrier> barriers_after_upload;

        if(should_upload_vertex_buffer) {
            RhiResourceBarrier barrier_after_vertex_upload = {};
            barrier_after_vertex_upload.resource_to_barrier = cur_vertex_buffer;
            barrier_after_vertex_upload.access_before_barrier = ResourceAccess::MemoryWrite;
            barrier_after_vertex_upload.access_after_barrier = ResourceAccess::VertexAttributeRead;
            barrier_after_vertex_upload.old_state = ResourceState::CopyDestination;
            barrier_after_vertex_upload.new_state = ResourceState::VertexBuffer;
            barrier_after_vertex_upload.source_queue = QueueType::Graphics;
            barrier_after_vertex_upload.destination_queue = QueueType::Graphics;
            barrier_after_vertex_upload.buffer_memory_barrier.offset = 0;
            barrier_after_vertex_upload.buffer_memory_barrier.size = num_vertex_bytes_to_upload;
            barriers_after_upload.push_back(barrier_after_vertex_upload);
        }

        if(should_upload_index_buffer) {
            RhiResourceBarrier barrier_after_index_upload = {};
            barrier_after_index_upload.resource_to_barrier = cur_index_buffer;
            barrier_after_index_upload.access_before_barrier = ResourceAccess::MemoryWrite;
            barrier_after_index_upload.access_after_barrier = ResourceAccess::IndexRead;
            barrier_after_index_upload.old_state = ResourceState::CopyDestination;
            barrier_after_index_upload.new_state = ResourceState::IndexBuffer;
            barrier_after_index_upload.source_queue = QueueType::Graphics;
            barrier_after_index_upload.destination_queue = QueueType::Graphics;
            barrier_after_index_upload.buffer_memory_barrier.offset = 0;
            barrier_after_index_upload.buffer_memory_barrier.size = num_index_bytes_to_upload;
            barriers_after_upload.push_back(barrier_after_index_upload);
        }

        if(barriers_after_upload.is_empty()) {
            return;
        }

        cmds->resource_barriers(PipelineStage::Transfer, PipelineStage::VertexInput, barriers_after_upload);
    }

    ProceduralMesh::Buffers ProceduralMesh::get_buffers_for_frame(const uint8_t frame_idx) const {
        auto* vertex_buffer = vertex_buffers[frame_idx].get();
        auto* index_buffer = index_buffers[frame_idx].get();

        return {vertex_buffer, index_buffer};
    }
} // namespace nova::renderer
