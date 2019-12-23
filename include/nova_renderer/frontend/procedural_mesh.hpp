#pragma once

#include <array>
#include <cstdint>

#include "nova_renderer/constants.hpp"
#include "nova_renderer/memory/block_allocation_strategy.hpp"
#include "nova_renderer/rhi/device_memory_resource.hpp"
#include "nova_renderer/rhi/forward_decls.hpp"

namespace nova::renderer {
    namespace rhi {
        struct Buffer;

        class CommandList;
        class RenderEngine;
    } // namespace rhi

    /*!
     * \brief ProceduralMesh is a mesh which the user will modify every frame
     *
     * ProceduralMesh should _not_ be used if you're not going to update the mesh frequently. It stores four copies of the mesh data - once
     * in host memory, and three times in device memory (one for each in-flight frame)
     */
    class ProceduralMesh {
    public:
        ProceduralMesh() = default;

        /*!
         * \brief Creates a new procedural mesh which has the specified amount of space
         *
         * \param vertex_buffer_size The number of bytes the vertex buffer needs
         * \param index_buffer_size The number of bytes that the index buffer needs
         * \param device The device to create the buffers on
         */
        ProceduralMesh(uint64_t vertex_buffer_size, uint64_t index_buffer_size, rhi::RenderEngine* device);

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
        void record_commands_to_upload_data(rhi::CommandList* cmds, uint8_t frame_idx);

        /*!
         * \brief Returns the vertex and index buffer for the provided frame
         */
        [[nodiscard]] std::tuple<rhi::Buffer*, rhi::Buffer*> get_buffers_for_frame(uint8_t frame_idx) const;

    private:
        rhi::RenderEngine* device;

        std::array<rhi::Buffer*, NUM_IN_FLIGHT_FRAMES> vertex_buffers;
        std::array<rhi::Buffer*, NUM_IN_FLIGHT_FRAMES> index_buffers;

        rhi::Buffer* cached_vertex_buffer;
        rhi::Buffer* cached_index_buffer;

        uint64_t num_vertex_bytes_to_upload = 0;
        uint64_t num_index_bytes_to_upload = 0;

#ifdef NOVA_DEBUG
        uint64_t vertex_buffer_size;
        uint64_t index_buffer_size;
        std::unique_ptr<bvestl::polyalloc::BlockAllocationStrategy> device_memory_allocation_strategy;
        std::unique_ptr<bvestl::polyalloc::BlockAllocationStrategy> host_memory_allocation_strategy;
        std::unique_ptr<DeviceMemoryResource> device_buffers_memory;
        std::unique_ptr<DeviceMemoryResource> cached_buffers_memory;
#endif
    };
} // namespace nova::renderer
