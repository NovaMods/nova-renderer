#include "nova_renderer/frontend/procedural_mesh.hpp"

#include "nova_renderer/render_engine.hpp"

namespace nova::renderer {
    using namespace rhi;

    ProceduralMesh::ProceduralMesh(uint64_t vertex_buffer_size, uint64_t index_buffer_size, rhi::RenderEngine& render_engine)
        : render_engine(render_engine) {
        DeviceMemory* vertex_buffer_memory = render_engine.allocate_device_memory(vertex_buffer_size * 3,
                                                                                  MemoryUsage::LowFrequencyUpload,
                                                                                  ObjectType::Buffer);

        BufferCreateInfo vertex_create_info = {vertex_buffer_size, BufferUsage::VertexBuffer};

        for(uint32_t i = 0; i < vertex_buffers.size(); i++) {
            vertex_buffers
        }
    }
} // namespace nova::renderer
