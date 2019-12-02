#pragma once

#include <cstdint>
#include <array>

#include "nova_renderer/constants.hpp"

namespace nova::renderer {
    namespace rhi {
        class RenderEngine;
        class Buffer;
    }

    /*!
     * \brief ProceduralMesh is a mesh which the user will modify every frame
     *
     * ProceduralMesh should _not_ be used if you're not going to update the mesh frequently. It stores four copies of the mesh data - once
     * in host memory, and three times in device memory (one for each in-flight frame)
     */
    class ProceduralMesh {
    public:
        ProceduralMesh(uint64_t vertex_buffer_size, uint64_t index_buffer_size, rhi::RenderEngine& render_engine);

    private:
        rhi::RenderEngine& render_engine;

        std::array<rhi::Buffer*, NUM_IN_FLIGHT_FRAMES> vertex_buffers;
		std::array<rhi::Buffer*, NUM_IN_FLIGHT_FRAMES> index_buffers;

        rhi::Buffer* cached_vertex_buffer;
        rhi::Buffer* cached_index_buffer;
    };
} // namespace nova::renderer
