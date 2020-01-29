#include "nova_renderer/procedural_mesh.hpp"

#include "nova_renderer/rhi/device_memory_resource.hpp"
#include "nova_renderer/rhi/render_device.hpp"
#include "nova_renderer/util/logger.hpp"

#include "../util/memory_utils.hpp"

using namespace nova::mem;

namespace nova::renderer {
    using namespace rhi;

    ProceduralMesh::ProceduralMesh(const uint64_t vertex_buffer_size, const uint64_t index_buffer_size, RenderDevice* device)
        : device(device)
#ifdef NOVA_DEBUG
          ,
          vertex_buffer_size(vertex_buffer_size),
          index_buffer_size(index_buffer_size)
#endif
    {
        const auto aligned_vertex_buffer_size = align(vertex_buffer_size, 256);
        const auto aligned_index_buffer_size = align(index_buffer_size, 256);
        const auto host_memory_size = aligned_vertex_buffer_size + aligned_index_buffer_size;
        const auto device_memory_size = host_memory_size * 3;

        allocator = &rx::memory::g_system_allocator;

        device_memory_allocation_strategy = allocator->create<BlockAllocationStrategy>(allocator, device_memory_size, 0_b);
        host_memory_allocation_strategy = allocator->create<BlockAllocationStrategy>(allocator, host_memory_size, 0_b);

        // TODO: Don't allocate a separate DeviceMemory for each procedural mesh
        device->allocate_device_memory(device_memory_size.b_count(), MemoryUsage::LowFrequencyUpload, ObjectType::Buffer, allocator)
            .map([&](DeviceMemory* memory) {
                device_buffers_memory = allocator->create<DeviceMemoryResource>(memory, device_memory_allocation_strategy);

                const auto vertex_create_info = BufferCreateInfo{vertex_buffer_size, BufferUsage::VertexBuffer};
                for(uint32_t i = 0; i < vertex_buffers.size(); i++) {
                    vertex_buffers[i] = device->create_buffer(vertex_create_info, *device_buffers_memory, allocator);
                }

                const auto index_create_info = BufferCreateInfo{index_buffer_size, BufferUsage::IndexBuffer};
                for(uint32_t i = 0; i < index_buffers.size(); i++) {
                    index_buffers[i] = device->create_buffer(index_create_info, *device_buffers_memory, allocator);
                }

                return true;
            })
            .on_error([](const ntl::NovaError& error) {
                NOVA_LOG(ERROR) << "Could not allocate device memory for procedural mesh. Error: " << error.to_string();
                // TODO: Propagate the error
            });

        device->allocate_device_memory(host_memory_size.b_count(), MemoryUsage::StagingBuffer, ObjectType::Buffer, allocator)
            .map([&](DeviceMemory* memory) {
                cached_buffers_memory = allocator->create<DeviceMemoryResource>(memory, host_memory_allocation_strategy);

                cached_vertex_buffer = device->create_buffer({vertex_buffer_size, BufferUsage::StagingBuffer},
                                                             *cached_buffers_memory,
                                                             allocator);
                cached_index_buffer = device->create_buffer({index_buffer_size, BufferUsage::StagingBuffer},
                                                            *cached_buffers_memory,
                                                            allocator);

                return true;
            })
            .on_error([](const ntl::NovaError& error) {
                NOVA_LOG(ERROR) << "Could not allocate host memory for procedural mesh. Error: " << error.to_string();
                // TODO: Propagate the error
            });
    }

    ProceduralMesh::~ProceduralMesh() {
        // TODO: Free the device memory with the RenderDevice
        // Requires support from the RHI

        allocator->destroy<DeviceMemoryResource>(device_buffers_memory);
        allocator->destroy<DeviceMemoryResource>(cached_buffers_memory);

        allocator->destroy<mem::BlockAllocationStrategy>(device_memory_allocation_strategy);
        allocator->destroy<mem::BlockAllocationStrategy>(host_memory_allocation_strategy);
    }

    void ProceduralMesh::set_vertex_data(const void* data, const uint64_t size) {
#ifdef NOVA_DEBUG
        if(size > vertex_buffer_size) {
            NOVA_LOG(ERROR) << "Cannot upload vertex data. There's only space for " << vertex_buffer_size << " bytes, you tried to upload "
                            << size << ". Truncating vertex data to fit";

            device->write_data_to_buffer(data, vertex_buffer_size, 0, cached_vertex_buffer);
            num_vertex_bytes_to_upload = vertex_buffer_size;

        } else {
#endif

            device->write_data_to_buffer(data, size, 0, cached_vertex_buffer);
            num_vertex_bytes_to_upload = size;

#ifdef NOVA_DEBUG
        }
#endif
    }

    void ProceduralMesh::set_index_data(const void* data, const uint64_t size) {
#ifdef NOVA_DEBUG
        if(size > index_buffer_size) {
            NOVA_LOG(ERROR) << "Cannot upload index data. There's only space for " << index_buffer_size << " bytes, you tried to upload "
                            << size << ". Truncating vertex data to fit";

            device->write_data_to_buffer(data, index_buffer_size, 0, cached_index_buffer);
            num_index_bytes_to_upload = index_buffer_size;

        } else {
#endif
            device->write_data_to_buffer(data, size, 0, cached_index_buffer);
            num_index_bytes_to_upload = size;

#ifdef NOVA_DEBUG
        }
#endif
    }

    void ProceduralMesh::record_commands_to_upload_data(CommandList* cmds, const uint8_t frame_idx) const {
        const bool should_upload_vertex_buffer = num_vertex_bytes_to_upload > 0;
        const bool should_upload_index_buffer = num_index_bytes_to_upload > 0;

        auto* cur_vertex_buffer = vertex_buffers[frame_idx];
        auto* cur_index_buffer = index_buffers[frame_idx];

        rx::vector<ResourceBarrier> barriers_before_upload;

        if(should_upload_vertex_buffer) {
            ResourceBarrier barrier_before_vertex_upload = {};
            barrier_before_vertex_upload.resource_to_barrier = cur_vertex_buffer;
            barrier_before_vertex_upload.access_before_barrier = ResourceAccess::VertexAttributeRead;
            barrier_before_vertex_upload.access_after_barrier = ResourceAccess::MemoryWrite;
            barrier_before_vertex_upload.old_state = ResourceState::VertexBuffer;
            barrier_before_vertex_upload.new_state = ResourceState::CopyDestination;
            barrier_before_vertex_upload.source_queue = QueueType::Graphics;
            barrier_before_vertex_upload.destination_queue = QueueType::Transfer;
            barrier_before_vertex_upload.buffer_memory_barrier.offset = 0;
            barrier_before_vertex_upload.buffer_memory_barrier.size = num_vertex_bytes_to_upload;

            barriers_before_upload.push_back(barrier_before_vertex_upload);
        }

        if(should_upload_index_buffer) {
            ResourceBarrier barrier_before_index_upload = {};
            barrier_before_index_upload.resource_to_barrier = cur_index_buffer;
            barrier_before_index_upload.access_before_barrier = ResourceAccess::IndexRead;
            barrier_before_index_upload.access_after_barrier = ResourceAccess::MemoryWrite;
            barrier_before_index_upload.old_state = ResourceState::IndexBuffer;
            barrier_before_index_upload.new_state = ResourceState::CopyDestination;
            barrier_before_index_upload.source_queue = QueueType::Graphics;
            barrier_before_index_upload.destination_queue = QueueType::Transfer;
            barrier_before_index_upload.buffer_memory_barrier.offset = 0;
            barrier_before_index_upload.buffer_memory_barrier.size = num_vertex_bytes_to_upload;

            barriers_before_upload.push_back(barrier_before_index_upload);
        }

        if(barriers_before_upload.is_empty()) {
            return;
        }

        cmds->resource_barriers(PipelineStage::BottomOfPipe, PipelineStage::Transfer, barriers_before_upload);

        if(should_upload_vertex_buffer) {
            cmds->copy_buffer(cur_vertex_buffer, 0, cached_vertex_buffer, 0, num_vertex_bytes_to_upload);
        }

        if(should_upload_index_buffer) {
            cmds->copy_buffer(cur_index_buffer, 0, cached_index_buffer, 0, num_index_bytes_to_upload);
        }

        rx::vector<ResourceBarrier> barriers_after_upload;

        if(should_upload_vertex_buffer) {
            ResourceBarrier barrier_after_vertex_upload = {};
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
            ResourceBarrier barrier_after_index_upload = {};
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

        cmds->resource_barriers(PipelineStage::Transfer, PipelineStage::TopOfPipe, barriers_after_upload);
    }

    ProceduralMesh::Buffers ProceduralMesh::get_buffers_for_frame(const uint8_t frame_idx) const {
        auto* vertex_buffer = vertex_buffers[frame_idx];
        auto* index_buffer = index_buffers[frame_idx];

        return {vertex_buffer, index_buffer};
    }
} // namespace nova::renderer
