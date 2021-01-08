#pragma once

#include <rx/core/array.h>
#include <rx/core/concepts/no_copy.h>
#include <rx/core/string.h>
#include <stdint.h>

#include "nova_renderer/constants.hpp"
#include "nova_renderer/rhi/forward_decls.hpp"

namespace nova::renderer {
    /*!
     * \brief ProceduralMesh is a mesh which the user will modify every frame
     *
     * ProceduralMesh should _not_ be used if you're not going to update the mesh frequently. It stores four copies of the mesh data - once
     * in host memory, and three times in device memory (one for each in-flight frame)
     */
    class ProceduralMesh : rx::concepts::no_copy {
    public:
        struct Buffers {
            rhi::RhiBuffer* vertex_buffer;
            rhi::RhiBuffer* index_buffer;
        };

        ProceduralMesh() = default;

        /*!
         * \brief Creates a new procedural mesh which has the specified amount of space
         *
         * \param vertex_buffer_size The number of bytes the vertex buffer needs
         * \param index_buffer_size The number of bytes that the index buffer needs
         * \param num_in_flight_frames Number of in-flight frames that this proc mesh supports
         * \param device The device to create the buffers on
         * \param name Name of this procedural mesh
         */
        ProceduralMesh(uint64_t vertex_buffer_size,
                       uint64_t index_buffer_size,
                       uint32_t num_in_flight_frames,
                       rhi::RenderDevice* device,
                       const std::string& name = "ProceduralMesh");

        ProceduralMesh(ProceduralMesh&& old) noexcept;
        ProceduralMesh& operator=(ProceduralMesh&& old) noexcept;

        ~ProceduralMesh() = default;

        /*!
         * \brief Sets the data to upload to the vertex buffer
         *
         * \param data A pointer to the start of the data
         * \param size The number of bytes to upload
         */
        void set_vertex_data(const void* data, uint64_t size);

        /*!
         * \brief Sets the data to upload to the index buffer
         *
         * \param data A pointer to the start of the data
         * \param size The number of bytes to upload
         */
        void set_index_data(const void* data, uint64_t size);

        /*!
         * \brief Records commands to copy the staging buffers to the device buffers for the specified frame
         *
         * Intent is that you call this method at the beginning or the frame that will use the buffers you're uploading to. It's a method on
         * this class mostly because writing accessors is hard
         *
         * \param cmds The command list to record commands into
         * \param frame_idx The index of the frame to write the data to
         */
        void record_commands_to_upload_data(rhi::RhiRenderCommandList* cmds, uint8_t frame_idx) const;

        /*!
         * \brief Returns the vertex and index buffer for the provided frame
         */
        [[nodiscard]] Buffers get_buffers_for_frame(uint8_t frame_idx) const;

    private:
        rhi::RenderDevice* device = nullptr;

        std::string name;

        std::vector<rhi::RhiBuffer*> vertex_buffers;
        std::vector<rhi::RhiBuffer*> index_buffers;

        rhi::RhiBuffer* cached_vertex_buffer;
        rhi::RhiBuffer* cached_index_buffer;

        uint64_t num_vertex_bytes_to_upload = 0;
        uint64_t num_index_bytes_to_upload = 0;

        rx::memory::allocator* allocator;

#ifdef NOVA_DEBUG
        uint64_t vertex_buffer_size;
        uint64_t index_buffer_size;
#endif
    };
} // namespace nova::renderer
