#include "nova_renderer/frontend/procedural_mesh.hpp"

#include "nova_renderer/render_engine.hpp"

#include "../memory/block_allocation_strategy.hpp"
#include "../memory/mallocator.hpp"
#include "../util/logger.hpp"
#include "../util/memory_utils.hpp"

namespace nova::renderer {
    using namespace rhi;

    ProceduralMesh::ProceduralMesh(uint64_t vertex_buffer_size, uint64_t index_buffer_size, rhi::RenderEngine& render_engine)
        : render_engine(render_engine) {
        const auto aligned_vertex_buffer_size = bvestl::polyalloc::align(vertex_buffer_size, 256);
        const auto aligned_index_buffer_size = bvestl::polyalloc::align(index_buffer_size, 256);
        const auto host_memory_size = aligned_vertex_buffer_size + aligned_index_buffer_size;
        const auto device_memory_size = host_memory_size * 3;

        // TODO: Don't allocate a separate device memory for each procedural mesh
        render_engine.allocate_device_memory(device_memory_size.b_count(), MemoryUsage::LowFrequencyUpload, ObjectType::Buffer)
            .map([&](DeviceMemory* memory) {
                // TODO: Find a good way to keep these around
                const auto
                    allocation_strategy = std::make_unique<bvestl::polyalloc::BlockAllocationStrategy>(new bvestl::polyalloc::Mallocator(),
                                                                                                       device_memory_size);
                auto memory_resource = DeviceMemoryResource(memory, allocation_strategy.get());

                const auto vertex_create_info = BufferCreateInfo{vertex_buffer_size, BufferUsage::VertexBuffer};
                for(auto& vertex_buffer : vertex_buffers) {
                    vertex_buffer = render_engine.create_buffer(vertex_create_info, memory_resource);
                }

                const auto index_create_info = BufferCreateInfo{index_buffer_size, BufferUsage::IndexBuffer};
                for(auto& index_buffer : index_buffers) {
                    index_buffer = render_engine.create_buffer(index_create_info, memory_resource);
                }

                return ntl::Result<bool>(true);
            })
            .on_error([](const ntl::NovaError& error) {
                NOVA_LOG(ERROR) << "Could not allocate device memory for procedural mesh. Error: " << error.to_string();
            });

        render_engine.allocate_device_memory(host_memory_size.b_count(), MemoryUsage::StagingBuffer, ObjectType::Buffer)
            .map([&](DeviceMemory* memory) {
                // TODO: Find a good way to keep these around
                const auto
                    allocation_strategy = std::make_unique<bvestl::polyalloc::BlockAllocationStrategy>(new bvestl::polyalloc::Mallocator(),
                                                                                                       host_memory_size);
                auto memory_resource = DeviceMemoryResource(memory, allocation_strategy.get());

                cached_vertex_buffer = render_engine.create_buffer({vertex_buffer_size, BufferUsage::StagingBuffer}, memory_resource);
                cached_index_buffer = render_engine.create_buffer({index_buffer_size, BufferUsage::StagingBuffer}, memory_resource);

                return ntl::Result<bool>(true);
            })
            .on_error([](const ntl::NovaError& error) {
                NOVA_LOG(ERROR) << "Could not allocate host memory for procedural mesh. Error: " << error.to_string();
            });
    }
} // namespace nova::renderer
